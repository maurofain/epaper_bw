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

# Font definitions: (font_id, name, max_chars)
# Based on src/main.cpp fontDefinitions[] array
FONT_DATABASE = [
    # Montserrat (1-3)
    (1, "montserrat_14", 12),
    (2, "montserrat_28", 5),
    (3, "montserrat_48", 2),
    # GoogleSans (4-11)
    (4, "GoogleSans10", 18),
    (5, "GoogleSans15", 15),
    (6, "GoogleSans20", 12),
    (7, "GoogleSans35", 6),
    (8, "GoogleSans50", 5),
    (9, "GoogleSans60", 4),
    (10, "GoogleSans100", 3),
    (11, "GoogleSans140", 2),
    # GoogleSansBold (12-15)
    (12, "GoogleSansBold40", 5),
    (13, "GoogleSansBold60", 4),
    (14, "GoogleSansBold100", 3),
    (15, "GoogleSansBold140", 2),
]


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


def select_font_for_text(text: str, use_bold: bool = False, use_montserrat: bool = False) -> tuple[int, str]:
    """
    Auto-select best font for text based on length and style.
    Returns (font_id, font_name).
    Tries to use the largest font that fits the text length.
    """
    text_len = len(text)
    
    # Select font family based on flags
    if use_montserrat:
        # Montserrat family (font 1-3)
        candidates = [(fid, fname, maxch) for fid, fname, maxch in FONT_DATABASE if 1 <= fid <= 3]
    elif use_bold:
        # GoogleSansBold family (font 12-15)
        candidates = [(fid, fname, maxch) for fid, fname, maxch in FONT_DATABASE if 12 <= fid <= 15]
    else:
        # GoogleSans family (font 4-11) - default
        candidates = [(fid, fname, maxch) for fid, fname, maxch in FONT_DATABASE if 4 <= fid <= 11]
    
    # Find largest font that fits (iterate backwards = largest to smallest)
    for fid, fname, maxch in reversed(candidates):
        if text_len <= maxch:
            return fid, fname
    
    # If nothing fits, use smallest available font
    return candidates[0][0], candidates[0][1]


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
    left_win.addstr(6, 4, "x - disconnect 0x00 0xEE")
    left_win.addstr(7, 4, "r - reboot 0x00 0xBB")
    left_win.addstr(8, 4, "t - testo auto")
    left_win.addstr(9, 4, "e - testo esteso")
    left_win.addstr(10, 4, "l - LED values")
    left_win.addstr(11, 4, "w - watch secondi")
    left_win.addstr(12, 4, "d - countdown (scegli font 1-12)")
    left_win.addstr(13, 4, "z - font demo (mostra tutti i font)")
    left_win.addstr(14, 4, "f - toggle log visibility")
    left_win.addstr(15, 4, "q - esci")
    
    current_line = 16
    if state["mode"] == "c":
        left_win.addstr(current_line, 2, "Comandi 0x00:")
        left_win.addstr(current_line + 1, 4, "0 - refresh totale")
        left_win.addstr(current_line + 2, 4, "1 - scanner ON + refresh")
        left_win.addstr(current_line + 3, 4, "2 - scanner ON")
        left_win.addstr(current_line + 4, 4, "3 - scanner OFF")
        left_win.addstr(current_line + 5, 4, "4 - tema normale")
        left_win.addstr(current_line + 6, 4, "5 - tema invertito")
        left_win.addstr(current_line + 7, 4, "b - reboot")
        left_win.addstr(current_line + 8, 4, "f - mostra logo")
        status_line = current_line + 10
    elif state["mode"] == "a":
        left_win.addstr(current_line, 2, "Font disponibili (15 totali):")
        left_win.addstr(current_line + 1, 2, "MONTSERRAT: 1-3 | GOOGLE SANS: 4-11 | BOLD: 12-15")
        status_line = current_line + 3
    elif state["mode"] in {"w", "w_active"}:
        font_info = state["ctx"].get("w_font", "?")
        left_win.addstr(current_line, 2, f"Watch: font {font_info} - qualsiasi tasto ferma")
        status_line = current_line + 2
    elif state["mode"] in {"d", "d_active"}:
        font_info = state["ctx"].get("d_font", "?") if state["mode"] == "d_active" else "?"
        left_win.addstr(current_line, 2, f"Countdown: font {font_info}")
        status_line = current_line + 2
    elif state["mode"] == "z_active":
        left_win.addstr(current_line, 2, "Font demo in corso... premi 'q' per fermare")
        status_line = current_line + 2
    else:
        left_win.addstr(current_line, 2, "Mods attivi: premi 'b' per toggle bold (ç)")
        status_line = current_line + 2
    
    left_win.addstr(status_line, 2, "Status:")
    left_win.addstr(status_line + 1, 4, state["status"][: left_win.getmaxyx()[1] - 6])
    left_win.addstr(rows - 7, 2, "Nota: § prima del testo = clear display")
    if state["active"]:
        left_win.addstr(rows - 6, 2, "Premi 'q' per tornare al menu principale")
    left_win.addstr(rows - 5, 2, "Prompt:")
    left_win.addstr(rows - 4, 4, state["prompt_text"][: left_win.getmaxyx()[1] - 6])
    left_win.addstr(rows - 3, 2, "> " + state["input_line"][: left_win.getmaxyx()[1] - 4])
    left_win.noutrefresh()

    right_win.erase()
    if state["show_logs"]:
        right_win.box()
        right_win.addstr(1, 2, "Ingresso seriale (solo testo)")
        log_start = max(0, len(log_lines) - (right_rows - 3))
        for idx, line in enumerate(log_lines[log_start:]):
            line_text = line[: right_cols - 4]
            right_win.addstr(2 + idx, 2, line_text)
    right_win.noutrefresh()
    curses.doupdate()


def set_command_state(state: dict, mode: str) -> None:
    # Preserve important context values before reset
    w_font = state["ctx"].get("w_font") if state["ctx"] else None
    w_text = state["ctx"].get("w_text") if state["ctx"] else None
    d_font = state["ctx"].get("d_font") if state["ctx"] else None
    d_text = state["ctx"].get("d_text") if state["ctx"] else None
    
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
    elif mode == "x":
        state["prompt_text"] = "Premi invio per disconnect dalla scheda"
        state["status"] = "Disconnect 0x00 0xEE"
    elif mode == "a":
        state["prompt_text"] = "Elenco font disponibili"
        state["status"] = "Font list"
    elif mode == "w":
        state["prompt_text"] = "Watch: Font 1-15 (a-d per bold) [9]"
        state["status"] = "Watch - scegli font"
        state["ctx"]["useBold"] = False
        state["step"] = 0
    elif mode == "w_active":
        # Restore preserved values
        if w_font is not None:
            state["ctx"]["w_font"] = w_font
        if w_text is not None:
            state["ctx"]["w_text"] = w_text
        font = state["ctx"].get("w_font", 9)
        state["prompt_text"] = f"Watch: font {font} - premi qualsiasi tasto per fermare"
        state["status"] = f"Watch attivo (font {font})"
        state["last_w_send"] = 0.0
    elif mode == "d":
        state["prompt_text"] = "Countdown: Font 1-15 (a-d per bold) [9]"
        state["status"] = "Countdown - scegli font"
        state["step"] = 0
    elif mode == "d_active":
        # Restore preserved values
        if d_font is not None:
            state["ctx"]["d_font"] = d_font
        if d_text is not None:
            state["ctx"]["d_text"] = d_text
        font = state["ctx"].get("d_font", 9)
        state["prompt_text"] = f"Countdown: font {font} - qualsiasi tasto ferma"
        state["status"] = f"Countdown attivo (font {font})"
        state["ctx"]["d_counter"] = 60
        state["ctx"]["d_inverted"] = False
        state["ctx"]["useBold"] = False
        state["last_d_send"] = 0.0
    elif mode == "t":
        state["prompt_text"] = "Testo (ç=bold, £=Montserrat, §=refresh) auto seleziona font"
        state["status"] = "Testo auto"
    elif mode == "e":
        state["prompt_text"] = "Font 1-15 [7] (vedi 'a' per lista completa)"
        state["status"] = "Testo esteso"
    elif mode == "l":
        state["prompt_text"] = "LED1 r,g,b [0,0,0]"
        state["status"] = "LED values"
    elif mode == "z":
        state["prompt_text"] = "Premi invio per avviare loop di tutti i font (font 30 per nomi, testo '19' nel font)"
        state["status"] = "Font demo"
        state["ctx"]["font_index"] = 0
        state["ctx"]["running"] = False


def parse_font_input(value: str) -> int | None:
    """Parse font input: 1-15 or a-d (maps to 12-15). Returns None if invalid."""
    value = value.strip().lower()
    if value.isdigit():
        font = int(value)
        if 1 <= font <= 15:
            return font
    elif value in {"a", "b", "c", "d"}:
        return 12 + ord(value) - ord("a")
    return None


def complete_command(state: dict) -> None:
    state["active"] = False
    state["mode"] = ""
    state["step"] = 0
    state["prompt_text"] = "Premi c/i/r/x/a/t/e/l/w/d/z o q"
    state["status"] = "Pronto"
    state["answers"] = []
    state["ctx"] = {}


def process_active_input(
    state: dict,
    ser: serial.Serial,
    log_lines: List[str],
    exec_log: BinaryIO | None = None,
) -> None:
    value_raw = state["input_line"].strip()
    value = value_raw.lower()
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
    elif mode == "x":
        if value != "":
            state["status"] = "Premi Invio per inviare disconnect"
            return
        packet = bytes([0x00, 0xEE])
        _, warning = send_packet(
            ser,
            packet,
            "CMD 0x00 0xEE (disconnect)",
            log_lines,
            exec_log=exec_log,
        )
        if warning:
            state["status"] = warning
        complete_command(state)
        return
    if mode == "t":
        payload = normalize_text(value_raw)
        if len(payload) > 0xFE:
            payload = payload[:0xFE]
            state["status"] = "Testo troncato a 254 byte"
        
        # Auto-select font based on text length
        # Extract prefix flags: ç (bold), £ (Montserrat), § (partial refresh)
        text_str = value_raw
        use_bold = text_str.startswith("ç") or text_str.startswith("\xE7") or "\\xE7" in text_str
        use_montserrat = text_str.startswith("£") or text_str.startswith("\xA3") or "\\xA3" in text_str
        
        font_id, font_name = select_font_for_text(value_raw, use_bold=use_bold, use_montserrat=use_montserrat)
        
        # Build prefix with appropriate modifiers
        prefix = ""
        if use_montserrat:
            prefix += "£"
        if use_bold:
            prefix += "ç"
        
        # Add payload with prefix
        final_payload = normalize_text(prefix + value_raw)
        if len(final_payload) > 0xFE:
            final_payload = final_payload[:0xFE]
            state["status"] = "Testo troncato a 254 byte"
        
        packet = bytes([0x01, len(final_payload)]) + final_payload
        _, warning = send_packet(
            ser,
            packet,
            f"TEXT AUTO (font {font_id}: {font_name})",
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
                font = 7
            elif value.isdigit() and 1 <= int(value) <= 15:
                font = int(value)
            else:
                state["status"] = "Font non valido, inserisci 1-15"
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
            payload = normalize_text(value_raw)
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

    if mode == "w":
        if step == 0:
            if value == "":
                font = 9
            else:
                font = parse_font_input(value)
                if font is None:
                    state["status"] = "Font non valido, inserisci 1-15 o a-d"
                    return
            state["ctx"]["w_font"] = font
            state["step"] = 1
            state["prompt_text"] = "Watch: inserisci testo da visualizzare in loop"
            state["input_line"] = ""
            return
        if step == 1:
            payload = normalize_text(value_raw)
            if len(payload) > 250:
                payload = payload[:250]
                state["status"] = "Testo troncato a 250 byte"
            state["ctx"]["w_text"] = payload
            # Clear display before starting watch (partial refresh)
            _, warning = send_packet(
                ser, bytes([0x00, 0x00]), "CMD display refresh (partial)", log_lines, exec_log=exec_log,
            )
            set_command_state(state, "w_active")
            return

    if mode == "d":
        if step == 0:
            if value == "":
                font = 9
            else:
                font = parse_font_input(value)
                if font is None:
                    state["status"] = "Font non valido, inserisci 1-15 o a-d"
                    return
            state["ctx"]["d_font"] = font
            state["step"] = 1
            state["prompt_text"] = "Countdown: premi invio per avviare)"
            state["input_line"] = ""
            return
        if step == 1:
            payload = normalize_text(value_raw)
            if len(payload) > 250:
                payload = payload[:250]
                state["status"] = "Testo troncato a 250 byte"
            state["ctx"]["d_text"] = payload
            # Clear display before starting countdown (partial refresh)
            _, warning = send_packet(
                ser, bytes([0x00, 0x00]), "CMD display refresh (partial)", log_lines, exec_log=exec_log,
            )
            set_command_state(state, "d_active")
            return

    if mode == "a":
        # Font list display - just complete and let draw_windows show it
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
    
    if mode == "z":
        if value != "":
            state["status"] = "Premi Invio per avviare il loop"
            return
        # Start font demo loop
        state["mode"] = "z_active"
        state["ctx"]["running"] = True
        state["ctx"]["font_index"] = 0
        state["ctx"]["last_send"] = 0.0
        state["status"] = "Font demo avviato..."
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
        "prompt_text": "Premi c/i/r/x/a/t/e/l/w/d/z/f o q",
        "input_line": "",
        "status": "Pronto",
        "last_tx": "",
        "rx_buffer": b"",
        "last_w_send": 0.0,
        "last_d_send": 0.0,
        "show_logs": True,
        "init_sent": False,
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
                line_text = to_ascii(raw_line)
                append_log(log_lines, f"RX: {line_text}")
                # Check for init completed message and auto-send init command
                if "[M] init completed" in line_text and not state["init_sent"]:
                    state["init_sent"] = True
                    _, _ = send_packet(
                        ser, bytes([0x00, 0xAA]), "AUTO INIT on boot", log_lines, exec_log=exec_log,
                    )
                    state["status"] = "Init automatico inviato"

        # Countdown mode: send user text with counter every 1s, invert at 15, reset at 0
        if state["mode"] == "d_active":
            now = time.time()
            if now - state["last_d_send"] >= 1.0:
                ctr = state["ctx"]["d_counter"]
                font = state["ctx"].get("d_font", 8)
                use_bold = state["ctx"].get("useBold", False)
                user_text = state["ctx"].get("d_text", "")
                prefix = "ç§" if use_bold else "§"
                text_with_ctr = user_text + f"\n{ctr:02d}" if user_text else str(ctr)
                payload = normalize_text(prefix + text_with_ctr)
                _, warning = send_packet(
                    ser, bytes([0x01, 0xFF, font, 100, 100]) + payload + bytes([0x00]),
                    f"COUNTDOWN {ctr} (font {font}{'+ bold' if use_bold else ''})", log_lines, exec_log=exec_log,
                )
                state["status"] = warning if warning else f"Countdown: {ctr}"
                state["last_d_send"] = now
                if ctr == 15 and not state["ctx"]["d_inverted"]:
                    send_packet(ser, bytes([0x00, 0x05]), "CMD tema invertito", log_lines, exec_log=exec_log)
                    state["ctx"]["d_inverted"] = True
                if ctr == 0:
                    send_packet(ser, bytes([0x00, 0x04]), "CMD tema normale", log_lines, exec_log=exec_log)
                    state["ctx"]["d_inverted"] = False
                    state["ctx"]["d_counter"] = 60
                else:
                    state["ctx"]["d_counter"] = ctr - 1

        # Font demo mode: cycle through all fonts displaying "19"
        if state["mode"] == "z_active" and state["ctx"].get("running", False):
            now = time.time()
            if now - state["ctx"].get("last_send", 0.0) >= 4.0:  # 4 seconds per font
                font_idx = state["ctx"]["font_index"]
                if font_idx < len(FONT_DATABASE):
                    font_id, font_name, _ = FONT_DATABASE[font_idx]
                    
                    # Clear display with full refresh (c0 command)
                    _, _ = send_packet(
                        ser, bytes([0x00, 0x00]),
                        f"Font demo: full clear", log_lines, exec_log=exec_log,
                    )
                    time.sleep(0.5)  # Delay for full refresh to complete
                    
                    # Send font name at top with Montserrat 14pt (font 1)
                    font_name_text = f"({font_id}) {font_name}"
                    name_payload = normalize_text(font_name_text)
                    _, _ = send_packet(
                        ser, bytes([0x01, 0xFF, 1, 10, 20]) + name_payload + bytes([0x00]),
                        f"Font name", log_lines, exec_log=exec_log,
                    )
                    time.sleep(0.1)
                    
                    # Send "19" with current font at center
                    digit_payload = normalize_text("19")
                    _, _ = send_packet(
                        ser, bytes([0x01, 0xFF, font_id, 100, 100]) + digit_payload + bytes([0x00]),
                        f"Font {font_id}: {font_name}", log_lines, exec_log=exec_log,
                    )
                    
                    state["ctx"]["font_index"] += 1
                    state["ctx"]["last_send"] = now
                    state["status"] = f"Font demo: {font_id}/15 ({font_name})"
                else:
                    # Demo completed
                    state["ctx"]["running"] = False
                    complete_command(state)
                    state["status"] = "Font demo completato"

        # Watch mode: send user text with seconds updated every 1s
        if state["mode"] == "w_active":
            now = time.time()
            if now - state["last_w_send"] >= 1.0:
                sec = time.localtime().tm_sec
                font = state["ctx"].get("w_font", 9)
                use_bold = state["ctx"].get("useBold", False)
                user_text = state["ctx"].get("w_text", "")
                prefix = "ç§" if use_bold else "§"
                text_with_sec = user_text + f"\n{sec:02d}s" if user_text else str(sec)
                payload = normalize_text(prefix + text_with_sec)
                _, warning = send_packet(
                    ser, bytes([0x01, 0xFF, font, 100, 100]) + payload + bytes([0x00]),
                    f"WATCH {sec:02d}s (font {font}{'+ bold' if use_bold else ''})", log_lines, exec_log=exec_log,
                )
                state["status"] = warning if warning else f"Watch: inviato {sec:02d}s"
                state["last_w_send"] = now

        try:
            ch = stdscr.get_wch()
        except curses.error:
            ch = None

        if ch is None:
            time.sleep(0.05)
            continue

        # In watch/countdown mode any key stops it; 'q' also exits the app
        if state["mode"] in {"w_active", "d_active"}:
            if isinstance(ch, str) and ch.lower() == "q":
                break
            complete_command(state)
            continue
        
        # In font demo mode 'q' returns to menu
        if state["mode"] == "z_active":
            if isinstance(ch, str) and ch.lower() == "q":
                state["ctx"]["running"] = False
                complete_command(state)
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
        elif isinstance(ch, str) and ch.lower() == "q" and state["active"]:
            # Cancel active command and return to main menu
            complete_command(state)
            state["input_line"] = ""
        elif isinstance(ch, str) and ch.lower() == "q" and not state["active"]:
            break
        elif isinstance(ch, str) and ch.lower() == "b" and state["mode"] in {"w_active", "d_active"}:
            # Toggle bold font (ç flag) in active modes
            state["ctx"]["useBold"] = not state["ctx"].get("useBold", False)
            state["status"] = f"Bold: {'ON' if state['ctx']['useBold'] else 'OFF'} (font {state['ctx'].get('d_font', state['ctx'].get('w_font', '?'))})"
            state["last_d_send"] = time.time() - 1.0  # Force immediate send on next cycle
            state["last_w_send"] = time.time() - 1.0
        elif isinstance(ch, str) and ch.lower() == "f" and not state["active"]:
            # Toggle log visibility
            state["show_logs"] = not state["show_logs"]
            state["status"] = f"Log visibility: {'ON' if state['show_logs'] else 'OFF'}"
        elif isinstance(ch, str) and not state["active"] and ch.lower() in {"c", "i", "r", "x", "a", "t", "e", "l", "w", "d", "z"}:
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
        font_str = input("Scegli font da 1 a 15 [default 7] (vedi comandi per lista): ").strip()
        if font_str == "":
            font = 7
            break
        font = parse_font_input(font_str)
        if font is not None:
            break
        if font_str.isdigit() and 1 <= int(font_str) <= 15:
            font = int(font_str)
            break
        print("Valore non valido, inserisci 1-15.")
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
