#!/usr/bin/env python3
"""
Android/Termux bridge scaffold.

Reads controller input in a future revision and sends simple movement commands
to the XIAO SAMD21 over USB serial.
"""

from __future__ import annotations

import argparse
import time


def build_command(dx: int, dy: int, buttons: int) -> bytes:
    return f"M {dx} {dy} {buttons}\n".encode("ascii")


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--test", action="store_true", help="print test commands instead of using serial")
    args = parser.parse_args()

    if not args.test:
        raise SystemExit("Serial output is not implemented yet. Use --test for now.")

    pattern = [(5, 0, 0), (-5, 0, 0), (0, 5, 0), (0, -5, 0), (0, 0, 1), (0, 0, 0)]
    for dx, dy, buttons in pattern:
        print(build_command(dx, dy, buttons).decode("ascii").strip())
        time.sleep(0.25)

    return 0


if __name__ == "__main__":
    raise SystemExit(main())

