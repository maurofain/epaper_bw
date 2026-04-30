#!/usr/bin/env python3
"""
RS232 protocol tester for the EpaperQr board.

This tool sends commands according to docs/RS232_protocol.md.
It asks for the serial port at startup (default /dev/ttyACM0) and retries until it can open the port.

Menu options:
  c - protocol commands (0x00)
  t - text with automatic font selection (0x01 len 0-0xFE)
  e - extended text with explicit font and position (0x01 len=0xFF)
  l - LED control values (0x02)
  q - quit

Dependencies:
  pip install pyserial
"""

from __future__ import annotations

import curses
import re
import sys
import time
from typing import BinaryIO, List

try:
    import serial
    from serial.serialutil import SerialException
except ImportError:
    print("Errore: modulo 'pyserial' non installato. Esegui: pip install pyserial", file=sys.stderr)
    sys.exit(1)


DEFAULT_PORT = "/dev/ttyACM0"
DEFAULT_BAUD = 9600
READ_TIMEOUT = 0.05
WRITE_TIMEOUT = 0.5
MAX_LOG_LINES = 200


def to_hex(data: bytes) -> str:
    return " ".join(f"{byte:02X}" for byte in data)


def to_ascii(data: bytes) -> str:
    chars: List[str] = []
    for b in data:
        if 32 <= b <= 126:
            chars.append(chr(b))
        elif b in (9,):
            chars.append(" ")
        else:
            chars.append(".")
    return "".join(chars)


def split_serial_lines(data: bytes) -> tuple[List[bytes], bytes]:
    if not data:
        return [], b""

    parts: List[bytes] = re.split(rb"[\r\n]+", data)
    if len(parts) == 1:
        return [], data

    if data.endswith(b"\r") or data.endswith(b"\n"):
        return [part for part in parts if part], b""

    return [part for part in parts[:-1] if part], parts[-1]


def normalize_text(value: str) -> bytes:
    value = value.replace("\\n", "\r")
    return value.encode("latin-1", errors="replace")


def prompt_port() -> str:
    while True:
        answer = input(f"Porta seriale [{DEFAULT_PORT}]: ").strip()
        port = answer or DEFAULT_PORT
        try:
            with serial.Serial(
                port=port,
                baudrate=DEFAULT_BAUD,
                bytesize=serial.EIGHTBITS,
                parity=serial.PARITY_NONE,
                stopbits=serial.STOPBITS_ONE,
                timeout=READ_TIMEOUT,
                write_timeout=WRITE_TIMEOUT,
                xonxoff=False,
                rtscts=False,
                dsrdtr=False,
            ) as ser:
                print(f"Porta valida: {port}")
            return port
        except SerialException as exc:
            print(f"Impossibile aprire {port}: {exc}")
            print("Riprovare.")


def open_serial(port: str) -> serial.Serial:
    return serial.Serial(
        port=port,
        baudrate=DEFAULT_BAUD,
        bytesize=serial.EIGHTBITS,
        parity=serial.PARITY_NONE,
        stopbits=serial.STOPBITS_ONE,
        timeout=READ_TIMEOUT,
        write_timeout=WRITE_TIMEOUT,
        xonxoff=False,
        rtscts=False,
        dsrdtr=False,
    )


def append_exec_log(exec_log: BinaryIO | None, data: bytes) -> None:
    if exec_log is None:
        return
    exec_log.write(data)
    exec_log.flush()


def read_response(ser: serial.Serial, exec_log: BinaryIO | None = None) -> bytes:
    chunks: List[bytes] = []
    while True:
        chunk = ser.read(256)
        if not chunk:
            break
        append_exec_log(exec_log, chunk)
        chunks.append(chunk)
    return b"".join(chunks)


def append_log(log_lines: List[str], message: str) -> None:
    timestamp = time.strftime("%H:%M:%S")
    log_lines.append(f"[{timestamp}] {message}")
    if len(log_lines) > MAX_LOG_LINES:
        del log_lines[: len(log_lines) - MAX_LOG_LINES]


def send_packet(
    ser: serial.Serial,
    packet: bytes,
    label: str,
    log_lines: List[str] | None = None,
    exec_log: BinaryIO | None = None,
) -> tuple[bytes, str | None]:
    message = f"TX {label}: len={len(packet)} hex=[{to_hex(packet)}]"
    if log_lines is None:
        print(message)
    warning: str | None = None
    try:
        written = ser.write(packet)
        ser.flush()
    except (SerialException, OSError) as exc:
        warning = f"Errore scrittura seriale: {exc}"
        if log_lines is None:
            print(warning)
        return b"", warning

    if written != len(packet):
        warning = f"AVVISO: scritto solo {written}/{len(packet)} byte"
        if log_lines is None:
            print(warning)
    try:
        response = read_response(ser, exec_log)
    except (SerialException, OSError) as exc:
        warning = f"Errore lettura seriale: {exc}"
        if log_lines is None:
            print(warning)
        return b"", warning

    if response and log_lines is not None:
        append_log(log_lines, f"RX: {to_ascii(response)}")
    elif response:
        print(f"RX {label}: {to_ascii(response)}")
    return response, warning


def ensure_prompt_line(prompt: str) -> str:
    return prompt[:80]


def draw_windows(
    left_win: curses.window,
    right_win: curses.window,
    state: dict,
    log_lines: List[str],
) -> None:
    rows, cols = left_win.getmaxyx()
    right_rows, right_cols = right_win.getmaxyx()

    left_win.erase()
    left_win.box()
    left_win.addstr(1, 2, "EpaperQr Protocol Tester")
    left_win.addstr(3, 2, "Menu:")
    left_win.addstr(4, 4, "c - comandi 0x00")
    left_win.addstr(5, 4, "i - init 0x00 0xAA")
    left_win.addstr(6, 4, "r - reboot 0x00 0xBB")
    left_win.addstr(7, 4, "t - testo auto")
    left_win.addstr(8, 4, "e - testo esteso")
    left_win.addstr(9, 4, "l - LED values")
    left_win.addstr(10, 4, "q - esci")
    if state["mode"] == "c":
        left_win.addstr(12, 2, "Comandi 0x00:")
        left_win.addstr(13, 4, "0 - refresh totale")
        left_win.addstr(14, 4, "1 - scanner ON + refresh")
        left_win.addstr(15, 4, "2 - scanner ON")
        left_win.addstr(16, 4, "3 - scanner OFF")
        left_win.addstr(17, 4, "4 - tema normale (bianco/nero)")
        left_win.addstr(18, 4, "5 - tema invertito (nero/bianco)")
        left_win.addstr(19, 4, "b - reboot")
        left_win.addstr(20, 4, "f - mostra logo")
        status_line = 21
    else:
        status_line = 11
    left_win.addstr(status_line, 2, "Status:")
    left_win.addstr(status_line + 1, 4, state["status"][: left_win.getmaxyx()[1] - 6])
    left_win.addstr(rows - 7, 2, "Nota: § prima del testo = clear display")
    left_win.addstr(rows - 5, 2, "Prompt:")
    left_win.addstr(rows - 4, 4, state["prompt_text"][: left_win.getmaxyx()[1] - 6])
    left_win.addstr(rows - 3, 2, "> " + state["input_line"][: left_win.getmaxyx()[1] - 4])
    left_win.noutrefresh()

    right_win.erase()
    right_win.box()
    right_win.addstr(1, 2, "Ingresso seriale (solo testo)")
    log_start = max(0, len(log_lines) - (right_rows - 3))
    for idx, line in enumerate(log_lines[log_start:]):
        line_text = line[: right_cols - 4]
        right_win.addstr(2 + idx, 2, line_text)
    right_win.noutrefresh()
    curses.doupdate()


def set_command_state(state: dict, mode: str) -> None:
    state["active"] = True
    state["mode"] = mode
    state["step"] = 0
    state["answers"] = []
    state["ctx"] = {}
    if mode == "c":
        state["prompt_text"] = "Seleziona comando 0-5/b/f"
        state["status"] = "Comando 0x00"
    elif mode == "i":
        state["prompt_text"] = "Premi invio per inviare init"
        state["status"] = "Init 0x00 0xAA"
    elif mode == "r":
        state["prompt_text"] = "Premi invio per reboot"
        state["status"] = "Reboot 0x00 0xBB"
    elif mode == "t":
        state["prompt_text"] = "Inserisci testo (usa \\n per CR)"
        state["status"] = "Testo auto"
    elif mode == "e":
        state["prompt_text"] = "Font 1-6 [4]"
        state["status"] = "Testo esteso"
    elif mode == "l":
        state["prompt_text"] = "LED1 r,g,b [0,0,0]"
        state["status"] = "LED values"


def complete_command(state: dict) -> None:
    state["active"] = False
    state["mode"] = ""
    state["step"] = 0
    state["prompt_text"] = "Premi c/t/e/l o q"
    state["status"] = "Pronto"
    state["answers"] = []
    state["ctx"] = {}


def process_active_input(
    state: dict,
    ser: serial.Serial,
    log_lines: List[str],
    exec_log: BinaryIO | None = None,
) -> None:
    value = state["input_line"].strip().lower()
    mode = state["mode"]
    step = state["step"]

    if mode == "c":
        if value not in {"0", "1", "2", "3", "4", "5", "b", "f"}:
            state["status"] = "Scelta non valida, digita 0-5, b o f"
            return
        if value == "b":
            packet = bytes([0x00, 0xBB])
            label = "CMD 0x00 0xBB"
        elif value == "f":
            packet = bytes([0x00, 0xFF])
            label = "CMD 0x00 0xFF"
        else:
            packet = bytes([0x00, int(value)])
            label = f"CMD 0x00 0x{int(value):02X}"
        _, warning = send_packet(
            ser,
            packet,
            label,
            log_lines,
            exec_log=exec_log,
        )
        if warning:
            state["status"] = warning
        complete_command(state)
        return
    elif mode == "i":
        if value != "":
            state["status"] = "Premi Invio per inviare init"
            return
        packet = bytes([0x00, 0xAA])
        _, warning = send_packet(
            ser,
            packet,
            "CMD 0x00 0xAA",
            log_lines,
            exec_log=exec_log,
        )
        if warning:
            state["status"] = warning
        complete_command(state)
        return
    elif mode == "r":
        if value != "":
            state["status"] = "Premi Invio per inviare reboot"
            return
        packet = bytes([0x00, 0xBB])
        _, warning = send_packet(
            ser,
            packet,
            "CMD 0x00 0xBB",
            log_lines,
            exec_log=exec_log,
        )
        if warning:
            state["status"] = warning
        complete_command(state)
        return
    if mode == "t":
        payload = normalize_text(value)
        if len(payload) > 0xFE:
            payload = payload[:0xFE]
            state["status"] = "Testo troncato a 254 byte"
        packet = bytes([0x01, len(payload)]) + payload
        _, warning = send_packet(
            ser,
            packet,
            "TEXT AUTO",
            log_lines,
            exec_log=exec_log,
        )
        if warning:
            state["status"] = warning
        complete_command(state)
        return

    if mode == "e":
        if step == 0:
            if value == "":
                font = 4
            elif value.isdigit() and 1 <= int(value) <= 6:
                font = int(value)
            else:
                state["status"] = "Font non valido, inserisci 1-6"
                return
            state["ctx"]["font"] = font
            state["step"] = 1
            state["prompt_text"] = "Posizione x,y [100,100]"
            state["input_line"] = ""
            return
        if step == 1:
            if value == "":
                pos_x, pos_y = 100, 100
            else:
                try:
                    x_str, y_str = [token.strip() for token in value.split(",", 1)]
                    pos_x = max(0, min(255, int(x_str)))
                    pos_y = max(0, min(255, int(y_str)))
                except Exception:
                    state["status"] = "Posizione non valida"
                    return
            state["ctx"]["pos_x"] = pos_x
            state["ctx"]["pos_y"] = pos_y
            state["step"] = 2
            state["prompt_text"] = "Testo (usa \\n per CR)"
            state["input_line"] = ""
            return
        if step == 2:
            payload = normalize_text(value)
            if len(payload) > 250:
                payload = payload[:250]
                state["status"] = "Testo troncato a 250 byte"
            packet = (
                bytes(
                    [0x01, 0xFF, state["ctx"]["font"], state["ctx"]["pos_x"], state["ctx"]["pos_y"]]
                )
                + payload
                + bytes([0x00])
            )
            _, warning = send_packet(
                ser,
                packet,
                "TEXT EXTENDED",
                log_lines,
                exec_log=exec_log,
            )
            if warning:
                state["status"] = warning
            complete_command(state)
            return

    if mode == "l":
        try:
            if value == "":
                rgb = [0, 0, 0]
            else:
                rgb = [int(token.strip()) for token in value.split(",")]
            if len(rgb) != 3 or any(not 0 <= v <= 255 for v in rgb):
                raise ValueError()
        except ValueError:
            state["status"] = "RGB non valido, usa r,g,b"
            return
        state["answers"].extend(rgb)
        next_led = len(state["answers"]) // 3 + 1
        if next_led <= 4:
            state["step"] += 1
            state["prompt_text"] = f"LED{next_led} r,g,b [0,0,0]"
            state["input_line"] = ""
            return
        packet = bytes([0x02] + state["answers"])
        _, warning = send_packet(
            ser,
            packet,
            "LED VALUES",
            log_lines,
            exec_log=exec_log,
        )
        if warning:
            state["status"] = warning
        complete_command(state)
        return


def run_curses_interface(
    stdscr: curses.window,
    ser: serial.Serial,
    exec_log: BinaryIO | None = None,
) -> None:
    curses.curs_set(1)
    stdscr.nodelay(True)
    stdscr.keypad(True)
    curses.noecho()

    state = {
        "active": False,
        "mode": "",
        "step": 0,
        "answers": [],
        "ctx": {},
        "prompt_text": "Premi c/t/e/l o q",
        "input_line": "",
        "status": "Pronto",
        "last_tx": "",
        "rx_buffer": b"",
    }
    log_lines: List[str] = []

    rows, cols = stdscr.getmaxyx()
    left_width = max(20, cols * 20 // 100)
    right_width = max(20, cols - left_width - 1)
    left_win = curses.newwin(rows, left_width, 0, 0)
    right_win = curses.newwin(rows, right_width, 0, left_width + 1)

    while True:
        draw_windows(left_win, right_win, state, log_lines)

        # Read serial in background.
        chunk = ser.read(256)
        if chunk:
            append_exec_log(exec_log, chunk)
            state["rx_buffer"] += chunk
            lines, remainder = split_serial_lines(state["rx_buffer"])
            state["rx_buffer"] = remainder
            for raw_line in lines:
                append_log(log_lines, f"RX: {to_ascii(raw_line)}")

        try:
            ch = stdscr.get_wch()
        except curses.error:
            ch = None

        if ch is None:
            time.sleep(0.05)
            continue

        if isinstance(ch, str) and ch == "\n":
            if state["active"]:
                try:
                    process_active_input(state, ser, log_lines, exec_log=exec_log)
                except Exception as exc:
                    append_log(log_lines, f"Errore comando: {exc}")
                    state["status"] = "Errore comando"
                state["input_line"] = ""
        elif isinstance(ch, str) and ch in {"\x08", "\x7f"}:
            state["input_line"] = state["input_line"][:-1]
        elif isinstance(ch, str) and ch.lower() == "q" and not state["active"]:
            break
        elif isinstance(ch, str) and not state["active"] and ch.lower() in {"c", "i", "r", "t", "e", "l"}:
            set_command_state(state, ch.lower())
            state["input_line"] = ""
        elif isinstance(ch, str) and ch == "\t":
            pass
        elif isinstance(ch, str) and (32 <= ord(ch) <= 255):
            state["input_line"] += ch
        elif ch == curses.KEY_BACKSPACE:
            state["input_line"] = state["input_line"][:-1]

    curses.echo()
    return


def text_auto_menu(ser: serial.Serial) -> None:
    print("\n-- Test testo con scelta automatica del font (0x01 len 0-0xFE) --")
    user_text = input("Inserisci testo (usa \\n per inviare CR): ").strip()
    payload = normalize_text(user_text)
    if len(payload) > 0xFE:
        print(f"Testo troppo lungo ({len(payload)}), verrà troncato a 254 byte.")
        payload = payload[:0xFE]
    packet = bytes([0x01, len(payload)]) + payload
    send_packet(ser, packet, "TEXT AUTO")


def text_extended_menu(ser: serial.Serial) -> None:
    print("\n-- Test testo esteso con font e posizione (0x01 0xFF ...) --")
    while True:
        font_str = input("Scegli font da 1 a 6 [default 4]: ").strip()
        if font_str == "":
            font = 4
            break
        if font_str.isdigit() and 1 <= int(font_str) <= 6:
            font = int(font_str)
            break
        print("Valore non valido, inserisci 1-6.")
    pos_x = 100
    pos_y = 100
    pos_input = input(f"Posizione centrale [{pos_x},{pos_y}] (Formato x,y): ").strip()
    if pos_input:
        try:
            x_str, y_str = [token.strip() for token in pos_input.split(",", 1)]
            pos_x = max(0, min(255, int(x_str)))
            pos_y = max(0, min(255, int(y_str)))
        except Exception:
            print("Valore posizione non valido, si usa 100,100.")
            pos_x = 100
            pos_y = 100
    user_text = input("Inserisci testo (usa \\n per inviare CR): ").strip()
    payload = normalize_text(user_text)
    if len(payload) > 250:
        print(f"Testo troppo lungo ({len(payload)}), verrà troncato a 250 byte.")
        payload = payload[:250]
    packet = bytes([0x01, 0xFF, font, pos_x, pos_y]) + payload + bytes([0x00])
    send_packet(ser, packet, "TEXT EXTENDED")


def led_menu(ser: serial.Serial) -> None:
    print("\n-- Controllo LED WS2812B (0x02) --")
    values: List[int] = []
    for led_index in range(1, 5):
        while True:
            raw = input(f"RGB LED {led_index} (r,g,b) [default 0,0,0]: ").strip()
            if raw == "":
                values.extend([0, 0, 0])
                break
            try:
                parts = [int(token.strip()) for token in raw.split(",")]
                if len(parts) != 3 or any(not 0 <= v <= 255 for v in parts):
                    raise ValueError()
                values.extend(parts)
                break
            except ValueError:
                print("Inserisci tre valori tra 0 e 255 separati da virgola.")
    packet = bytes([0x02] + values)
    send_packet(ser, packet, "LED VALUES")


def main() -> None:
    print("=== EpaperQr RS232 Protocol Tester ===")
    port = prompt_port()
    try:
        with open_serial(port) as ser:
            with open("exec.log", "wb") as exec_log:
                curses.wrapper(run_curses_interface, ser, exec_log)
    except SerialException as exc:
        print(f"Errore seriale: {exc}", file=sys.stderr)
        sys.exit(1)


if __name__ == "__main__":
    main()
