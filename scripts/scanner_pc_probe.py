#!/usr/bin/env python3
"""
Test seriale scanner da PC usando la stessa sequenza comandi del firmware ESP.

Dipendenze:
  pip install pyserial

Esempio:
  python3 scripts/scanner_pc_probe.py --port /dev/ttyACM1
"""

from __future__ import annotations

import argparse
import os
import select
import sys
import termios
import time
import tty
from typing import Iterable, List, Sequence, Tuple

try:
    import serial
except ImportError:
    print("Errore: modulo 'pyserial' non installato. Esegui: pip install pyserial", file=sys.stderr)
    sys.exit(1)


START_SCAN_CMD = bytes([0x01, ord("T"), 0x04])
STOP_SCAN_CMD = bytes([0x01, ord("P"), 0x04])
QUERY_PRODUCT_INFO_WITH_AT = bytes([0x01, 0x40, 0x51, 0x52, 0x59, 0x53, 0x59, 0x53, 0x04])
QUERY_PRODUCT_INFO_NO_AT = bytes([0x01, 0x51, 0x52, 0x59, 0x53, 0x59, 0x53, 0x04])
QUERY_PRODUCT_INFO_ASCII_CR = b"@QRYSYS\r"
QUERY_PRODUCT_INFO_ASCII_CRLF = b"@QRYSYS\r\n"
QUERY_PRODUCT_INFO_ASCII_NO_AT_CRLF = b"QRYSYS\r\n"
INIT_SETUP_E1_CMD = b"@SETUPE1\r"
INIT_INTERF_0_CMD = b"@INTERF0\r"
INIT_SETUP_E0_CMD = b"@SETUPE0\r"

DEFAULT_BAUDS = [9600, 115200, 57600, 38400, 19200]


def parse_bauds(value: str) -> List[int]:
    result: List[int] = []
    for token in value.split(","):
        token = token.strip()
        if not token:
            continue
        try:
            baud = int(token)
        except ValueError as exc:
            raise argparse.ArgumentTypeError(f"Baud non valido: {token}") from exc
        if baud <= 0:
            raise argparse.ArgumentTypeError(f"Baud deve essere > 0: {token}")
        result.append(baud)
    if not result:
        raise argparse.ArgumentTypeError("Nessun baud valido specificato")
    return result


def to_hex(data: bytes) -> str:
    return " ".join(f"{byte:02X}" for byte in data)


def to_ascii(data: bytes) -> str:
    chars = []
    for b in data:
        if 32 <= b <= 126:
            chars.append(chr(b))
        elif b in (9, 10, 13):
            chars.append(" ")
        else:
            chars.append(".")
    return "".join(chars)


def read_until_timeout(ser: serial.Serial, timeout_s: float) -> bytes:
    deadline = time.monotonic() + timeout_s
    chunks: List[bytes] = []
    while time.monotonic() < deadline:
        chunk = ser.read(256)
        if chunk:
            chunks.append(chunk)
    return b"".join(chunks)


def send_and_read(
    ser: serial.Serial,
    label: str,
    payload: bytes,
    response_timeout_s: float,
) -> bytes:
    ser.reset_input_buffer()
    print(f"TX {label:<30} len={len(payload):2d} hex=[{to_hex(payload)}]")
    written = ser.write(payload)
    ser.flush()
    if written != len(payload):
        print(f"WARN TX parziale {label}: scritto={written} atteso={len(payload)}")

    rx = read_until_timeout(ser, response_timeout_s)
    if rx:
        print(f"RX {label:<30} len={len(rx):2d} hex=[{to_hex(rx)}] ascii=[{to_ascii(rx)}]")
    else:
        print(f"RX {label:<30} no response")
    return rx


def open_serial(port: str, baud: int) -> serial.Serial:
    ser = serial.Serial(
        port=port,
        baudrate=baud,
        bytesize=serial.EIGHTBITS,
        parity=serial.PARITY_NONE,
        stopbits=serial.STOPBITS_ONE,
        timeout=0.05,
        write_timeout=0.5,
        xonxoff=False,
        rtscts=False,
        dsrdtr=False,
    )

    # Su alcuni device CDC ACM le ioctl DTR/RTS possono bloccare (Errno 110).
    # In questo script non sono necessarie, quindi le lasciamo in stato di default.
    return ser


def build_listen_baud_plan(bauds: Sequence[int]) -> List[int]:
    unique_desc = sorted(set(bauds), reverse=True)
    if 115200 in unique_desc:
        unique_desc.remove(115200)
    return [115200] + unique_desc


class KeyboardReader:
    def __init__(self) -> None:
        self.enabled = False
        self.fd: int | None = None
        self.old_settings = None

    def __enter__(self) -> "KeyboardReader":
        if sys.stdin.isatty():
            self.enabled = True
            self.fd = sys.stdin.fileno()
            self.old_settings = termios.tcgetattr(self.fd)
            tty.setcbreak(self.fd)
        return self

    def __exit__(self, exc_type, exc, tb) -> None:
        if self.enabled and self.fd is not None and self.old_settings is not None:
            termios.tcsetattr(self.fd, termios.TCSADRAIN, self.old_settings)

    def read_key(self) -> str | None:
        if not self.enabled or self.fd is None:
            return None
        ready, _, _ = select.select([sys.stdin], [], [], 0)
        if not ready:
            return None
        try:
            data = os.read(self.fd, 1)
        except OSError:
            return None
        if not data:
            return None
        return data.decode(errors="ignore")


def run_listen_only(
    port: str,
    bauds: Sequence[int],
    listen_step_seconds: float,
    auto_baud: bool,
    start_115200: bool,
) -> bool:
    got_data = False
    if start_115200:
        plan = build_listen_baud_plan(bauds)
    else:
        plan = list(dict.fromkeys(bauds))

    if not auto_baud:
        plan = [plan[0]]

    print(f"LISTEN ONLY mode start: porta={port} piano_baud={plan} step={listen_step_seconds:.1f}s")
    print("Inquadra ora un QR davanti allo scanner...")
    if auto_baud:
        print("Controlli tastiera: premi un tasto qualsiasi per cambiare baud, 'q' per uscire")
    else:
        print("Controlli tastiera: premi 'q' per uscire")
    if not sys.stdin.isatty():
        print("INFO: terminale non interattivo, cambio baud da tastiera disabilitato")

    try:
        baud_index = 0
        while baud_index < len(plan):
            baud = plan[baud_index]
            ser: serial.Serial | None = None
            deadline = time.monotonic() + listen_step_seconds
            print(f"LISTEN ONLY: prova baud={baud} per {listen_step_seconds:.1f}s")
            try:
                ser = open_serial(port, baud)
                ser.reset_input_buffer()
                manual_switch = False
                with KeyboardReader() as kb:
                    while time.monotonic() < deadline:
                        key = kb.read_key()
                        if key:
                            if key.lower() == "q":
                                print("LISTEN ONLY: uscita richiesta da tastiera")
                                return got_data
                            if auto_baud:
                                manual_switch = True
                                print(f"LISTEN ONLY: cambio baud richiesto da tastiera ({baud} -> prossimo)")
                                break

                        chunk = ser.read(256)
                        if not chunk:
                            continue
                        got_data = True
                        print(f"RX SCAN baud={baud} len={len(chunk):2d} hex=[{to_hex(chunk)}] ascii=[{to_ascii(chunk)}]")
                        break

                if manual_switch and not got_data:
                    baud_index = (baud_index + 1) % len(plan)
                    continue

                if got_data:
                    break

                if not auto_baud:
                    break

                baud_index += 1
                print(f"LISTEN ONLY: nessun dato a {baud}, scendo di velocita'")

            except serial.SerialException as exc:
                print(f"ERRORE apertura/comunicazione su {port} @ {baud}: {exc}")
                baud_index += 1
                print("LISTEN ONLY: provo il baud successivo")
            finally:
                if ser is not None:
                    try:
                        ser.close()
                    except BaseException as exc:
                        print(f"WARN chiusura seriale su {port} @ {baud}: {exc}")

            if got_data:
                break

    except KeyboardInterrupt:
        print("LISTEN ONLY interrotto da utente")

    if got_data:
        print("LISTEN ONLY: dati ricevuti dallo scanner")
    else:
        print("LISTEN ONLY: nessun dato ricevuto")
    return got_data


def run_probe(
    port: str,
    bauds: Sequence[int],
    attempts: int,
    response_timeout_s: float,
    inter_cmd_ms: int,
    between_attempt_ms: int,
    with_init: bool,
) -> bool:
    init_commands: Sequence[Tuple[str, bytes]] = (
        ("INIT_SETUP_E1", INIT_SETUP_E1_CMD),
        ("INIT_INTERF_0", INIT_INTERF_0_CMD),
        ("INIT_SETUP_E0", INIT_SETUP_E0_CMD),
    )

    query_commands: Sequence[Tuple[str, bytes]] = (
        ("QUERY_PRODUCT_INFO_WITH_AT", QUERY_PRODUCT_INFO_WITH_AT),
        ("QUERY_PRODUCT_INFO_NO_AT", QUERY_PRODUCT_INFO_NO_AT),
        ("QUERY_PRODUCT_INFO_ASCII_CR", QUERY_PRODUCT_INFO_ASCII_CR),
        ("QUERY_PRODUCT_INFO_ASCII_CRLF", QUERY_PRODUCT_INFO_ASCII_CRLF),
        ("QUERY_PRODUCT_INFO_ASCII_NO_AT_CRLF", QUERY_PRODUCT_INFO_ASCII_NO_AT_CRLF),
    )

    any_response = False

    for attempt in range(1, attempts + 1):
        print(f"\n=== ATTEMPT {attempt}/{attempts} ===")
        for baud in bauds:
            print(f"\n--- PORT={port} BAUD={baud} ---")
            ser: serial.Serial | None = None
            try:
                ser = open_serial(port, baud)
                if with_init:
                    for label, payload in init_commands:
                        send_and_read(ser, label, payload, response_timeout_s)
                        time.sleep(inter_cmd_ms / 1000.0)
                else:
                    print("SKIP init commands (INIT_SETUP_E1, INIT_INTERF_0, INIT_SETUP_E0)")

                for label, payload in query_commands:
                    rx = send_and_read(ser, label, payload, response_timeout_s)
                    if rx:
                        any_response = True
                    time.sleep(inter_cmd_ms / 1000.0)

                # Comandi opzionali equivalenti al comportamento scanner on/off.
                send_and_read(ser, "START_SCAN", START_SCAN_CMD, response_timeout_s)
                time.sleep(inter_cmd_ms / 1000.0)
                send_and_read(ser, "STOP_SCAN", STOP_SCAN_CMD, response_timeout_s)

            except serial.SerialException as exc:
                print(f"ERRORE apertura/comunicazione su {port} @ {baud}: {exc}")
            finally:
                if ser is not None:
                    try:
                        ser.close()
                    except BaseException as exc:
                        print(f"WARN chiusura seriale su {port} @ {baud}: {exc}")

        if any_response:
            break

        if attempt < attempts:
            time.sleep(between_attempt_ms / 1000.0)

    return any_response


def build_arg_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(
        description="Probe scanner seriale da PC con stessa sequenza comandi usata nel firmware ESP"
    )
    parser.add_argument("--port", default="/dev/ttyACM1", help="Porta seriale (default: /dev/ttyACM1)")
    parser.add_argument(
        "--bauds",
        type=parse_bauds,
        default=DEFAULT_BAUDS,
        help="Lista baud separata da virgole (default: 9600,115200,57600,38400,19200)",
    )
    parser.add_argument("--attempts", type=int, default=3, help="Numero tentativi completi (default: 3)")
    parser.add_argument(
        "--response-timeout-ms",
        type=int,
        default=800,
        help="Timeout risposta per comando in ms (default: 800)",
    )
    parser.add_argument(
        "--inter-cmd-ms",
        type=int,
        default=20,
        help="Pausa tra comandi in ms (default: 20)",
    )
    parser.add_argument(
        "--between-attempt-ms",
        type=int,
        default=100,
        help="Pausa tra tentativi completi in ms (default: 100)",
    )
    parser.add_argument(
        "--with-init",
        action="store_true",
        help="Invia anche i comandi INIT_SETUP_E1/INIT_INTERF_0/INIT_SETUP_E0 (default: disabilitato)",
    )
    parser.add_argument(
        "--listen-only",
        action="store_true",
        help="Non invia comandi: ascolta solo i dati trasmessi dallo scanner",
    )
    parser.add_argument(
        "--listen-seconds",
        type=float,
        default=10.0,
        help="Durata ascolto in secondi per ogni baud in --listen-only (default: 10)",
    )
    parser.add_argument(
        "--listen-auto-baud",
        action="store_true",
        help="Abilita cambio automatico baud allo scadere di --listen-seconds",
    )
    parser.add_argument(
        "--listen-start-115200",
        action="store_true",
        help="Con --listen-auto-baud, forza inizio da 115200 e poi scala in discesa",
    )
    return parser


def main(argv: Sequence[str] | None = None) -> int:
    parser = build_arg_parser()
    args = parser.parse_args(argv)

    if args.attempts <= 0:
        print("Errore: --attempts deve essere > 0", file=sys.stderr)
        return 1

    print("Scanner PC probe start")
    print(f"Porta: {args.port}")
    print(f"Baud list: {args.bauds}")

    if args.listen_only:
        if args.listen_seconds <= 0:
            print("Errore: --listen-seconds deve essere > 0", file=sys.stderr)
            return 1
        got_scan_data = run_listen_only(
            port=args.port,
            bauds=args.bauds,
            listen_step_seconds=args.listen_seconds,
            auto_baud=args.listen_auto_baud,
            start_115200=args.listen_start_115200,
        )
        if got_scan_data:
            print("\nRISULTATO: scanner ha inviato dati")
            return 0
        print("\nRISULTATO: scanner non ha inviato dati")
        return 2

    got_response = run_probe(
        port=args.port,
        bauds=args.bauds,
        attempts=args.attempts,
        response_timeout_s=args.response_timeout_ms / 1000.0,
        inter_cmd_ms=args.inter_cmd_ms,
        between_attempt_ms=args.between_attempt_ms,
        with_init=args.with_init,
    )

    if got_response:
        print("\nRISULTATO: almeno una risposta ricevuta")
        return 0

    print("\nRISULTATO: nessuna risposta ricevuta")
    return 2


if __name__ == "__main__":
    sys.exit(main())
