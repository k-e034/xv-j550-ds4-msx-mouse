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
    parser.add_argument("--list-ports", action="store_true", help="list serial ports and exit")
    parser.add_argument("--print-only", action="store_true", help="print test commands instead of using serial")
    parser.add_argument("--command", action="append", help="send one command, can be specified multiple times")
    parser.add_argument("--repeat", type=int, default=1)
    parser.add_argument("--read-seconds", type=float, default=0.5, help="read replies for this many seconds after sending")
    args = parser.parse_args()

    if args.list_ports:
        try:
            from serial.tools import list_ports
        except ImportError:
            raise SystemExit("pyserial is required: pip install pyserial")

        ports = list(list_ports.comports())
        if not ports:
            print("No serial ports found.")
            return 0

        for port in ports:
            print("{}\t{}\t{}".format(port.device, port.description, port.hwid))
        return 0

    if args.command:
        commands = []
        for command in args.command:
            command = command.strip()
            if command:
                commands.append((command + "\n").encode("ascii"))
    else:
        pattern = [(5, 0, 0), (-5, 0, 0), (0, 5, 0), (0, -5, 0), (0, 0, 1), (0, 0, 0)]
        commands = [build_command(dx, dy, buttons) for dx, dy, buttons in pattern]

    if args.print_only:
        for _ in range(args.repeat):
            for command in commands:
                print(command.decode("ascii").strip())
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
            for command in commands:
                ser.write(command)
                ser.flush()
                print(command.decode("ascii").strip())
                deadline = time.time() + args.read_seconds
                while time.time() < deadline:
                    reply = ser.readline()
                    if reply:
                        print("< {}".format(reply.decode("utf-8", "replace").rstrip()))
                time.sleep(0.25)

    return 0


if __name__ == "__main__":
    raise SystemExit(main())
