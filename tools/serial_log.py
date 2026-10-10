"""Stream ESP32 serial output to stdout and append it verbatim to a log file.

Usage:
    uv run --with pyserial python tools/serial_log.py [--port P] [--out FILE] [--seconds N]

Resets the board on open (RTS pulse) so the log starts from boot.
"""

import argparse
import datetime as dt
import pathlib
import sys
import time

import serial

DEFAULT_PORT = "/dev/cu.usbserial-0001"


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("--port", default=DEFAULT_PORT)
    parser.add_argument("--baud", type=int, default=115200)
    parser.add_argument("--out", type=pathlib.Path)
    parser.add_argument("--seconds", type=float, help="stop after N seconds")
    parser.add_argument(
        "--send", action="append", default=[],
        help="line to send once the board is up (repeatable, sent in order)",
    )
    parser.add_argument("--no-reset", action="store_true")
    args = parser.parse_args()

    out = args.out or pathlib.Path("captures/raw") / (
        dt.datetime.now().strftime("session-%Y%m%d-%H%M%S.log")
    )
    out.parent.mkdir(parents=True, exist_ok=True)

    port = serial.Serial()
    port.port, port.baudrate, port.timeout = args.port, args.baud, 0.2
    port.dtr = port.rts = False  # don't hold the board in reset/boot on open
    port.open()
    if not args.no_reset:
        port.rts = True
        time.sleep(0.1)
        port.rts = False

    print(f"logging {args.port} -> {out}", file=sys.stderr, flush=True)
    deadline = time.time() + args.seconds if args.seconds else None
    pending = list(args.send)
    next_send = time.time() + (0.5 if args.no_reset else 2.0)  # wait for boot
    with out.open("ab") as log:
        while deadline is None or time.time() < deadline:
            if pending and time.time() >= next_send:
                port.write(pending.pop(0).encode() + b"\n")
                next_send = time.time() + 1.0
            chunk = port.read(4096)
            if chunk:
                log.write(chunk)
                log.flush()
                sys.stdout.write(chunk.decode(errors="replace"))
                sys.stdout.flush()


if __name__ == "__main__":
    main()
