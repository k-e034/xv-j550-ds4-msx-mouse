#!/usr/bin/env python3
"""Android/Termux bridge scaffold for the XIAO firmware.

The first useful mode is a serial smoke test. It sends deterministic movement
commands to the XIAO without requiring DUALSHOCK 4 input yet.
"""

import argparse
import time


def build_command(dx, dy, buttons):
    return "M {} {} {}\n".format(dx, dy, buttons).encode("ascii")


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--port", help="serial device path, for example /dev/ttyACM0")
    parser.add_argument("--baud", type=int, default=115200)
    parser.add_argument("--print-only", action="store_true", help="print test commands instead of using serial")
    parser.add_argument("--repeat", type=int, default=1)
    args = parser.parse_args()

    pattern = [(5, 0, 0), (-5, 0, 0), (0, 5, 0), (0, -5, 0), (0, 0, 1), (0, 0, 0)]

    if args.print_only:
        for _ in range(args.repeat):
            for dx, dy, buttons in pattern:
                print(build_command(dx, dy, buttons).decode("ascii").strip())
                time.sleep(0.25)
        return 0

    if not args.port:
        raise SystemExit("Use --port /dev/ttyACM0 or --print-only.")

    try:
        import serial
    except ImportError:
        raise SystemExit("pyserial is required: pip install pyserial")

    with serial.Serial(args.port, args.baud, timeout=1) as ser:
        time.sleep(2.0)
        for _ in range(args.repeat):
            for dx, dy, buttons in pattern:
                command = build_command(dx, dy, buttons)
                ser.write(command)
                ser.flush()
                print(command.decode("ascii").strip())
                time.sleep(0.25)

    return 0


if __name__ == "__main__":
    raise SystemExit(main())
