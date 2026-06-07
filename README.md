# XV-J550 DS4 MSX Mouse Emulator

DUALSHOCK 4 input on Android, translated through a Seeed Studio XIAO SAMD21 into MSX-compatible mouse signals for the XV-J550 mouse port.

This project exists as a practical substitute for increasingly expensive MSX-compatible mice such as MOS-1(G). It does not assume access to an original mouse for protocol capture.

## Goal

```text
DUALSHOCK 4
  -> Android
  -> USB serial
  -> Seeed Studio XIAO SAMD21
  -> GPIO-level MSX mouse compatible signals
  -> XV-J550 mouse port
```

## Hardware

Required:

- DUALSHOCK 4 controller
- Android 10 or later device
- USB Type-C OTG adapter
- Seeed Studio XIAO SAMD21
- USB cable for the XIAO
- DE-9 / D-Sub 9 connector for the XV-J550 mouse port
- Wires, resistors, and prototyping board
- Level shifting or open-collector interface circuitry

Useful but optional:

- Logic analyzer
- Multimeter

Still likely needed for the first safe hardware test:

- Resistors for protection and pull-ups/pull-downs
- Level shifting or open-collector buffer parts
- A slim DE-9 cable or compact connector that fits the recessed XV-J550 mouse port

Not required:

- USB-to-RS-232C serial cable
- RS-232C crossover cable
- Original MOS-1(G) mouse
- Other MSX-compatible mouse hardware

## Safety Notes

The XV-J550 mouse connector is treated here as an MSX-compatible mouse port, not as an RS-232C serial port.

Do not connect RS-232C voltage levels directly to the mouse port.

The XIAO SAMD21 uses 3.3 V GPIO and is not assumed to be 5 V tolerant. Use level conversion or an open-collector style interface before connecting to a 5 V mouse port.

## Repository Layout

```text
android/termux/          Android-side DS4 input bridge prototype
android/native-app/      Native Android DS4-to-XIAO bridge
docs/                    Protocol notes
firmware/                XIAO SAMD21 firmware
hardware/                Wiring and electrical notes
```

## Development Plan

1. Implement an MSX mouse signal emulator on the XIAO SAMD21.
2. Feed test `dx`, `dy`, and button values from a PC or Android serial terminal.
3. Verify XV-J550 cursor movement and button behavior.
4. Add Android-side DUALSHOCK 4 input capture.
5. Tune scaling, dead zones, axis direction, and timing.

## Current Status

The first end-to-end cursor movement test has succeeded.

```text
Windows SSH client
  -> Xperia / Termux
  -> USB OTG
  -> XIAO SAMD21
  -> BSS138 level converters
  -> XV-J550 mouse port
  -> visible cursor movement
```

The XIAO firmware accepts USB CDC commands and successfully responds to the
XV-J550 mouse-port strobe sequence. A Termux USB helper sent a left-click pulse
and an X-axis movement command, and the XV-J550 cursor moved.

Verified controls:

```text
M 1 0 0    -> left
M -1 0 0   -> right
M 0 1 0    -> up
M 0 -1 0   -> down
M 0 0 1    -> left button down
M 0 0 0    -> left button release
```

The native Android app has also been verified end to end:

```text
DUALSHOCK 4 over Bluetooth
  -> Xperia Android game-controller input
  -> native XV-J550 Mouse app
  -> XIAO USB CDC
  -> XV-J550 cursor and button input
```

On the tested Xperia, Android exposes the active DS4 input device to the app as
`Virtual`. This is expected on that device; DS4 movement and buttons work.

Right-click is connected and available as button bit 1 (`M 0 0 2`). According
to the XV-J550 manual, right-click has no function on the creation screen and
should be verified on the layout screen.

Start with:

- [Current next steps](docs/current-next-steps.md)
- [Wiring notes](hardware/wiring.md)
- [Protocol notes](docs/protocol-notes.md)

## Local Python Environment

A local Conda environment can be created from:

```text
conda env create -p .\.conda-env -f environment.yml
```

List visible serial ports:

```text
.\.conda-env\python.exe android\termux\ds4_to_xiao.py --list-ports
```

Send commands and read replies:

```text
.\.conda-env\python.exe android\termux\ds4_to_xiao.py --port COM3 --command S --command "M 5 0 0" --command S
```

## Related Projects

PS-LX10 / XV-J550 capture and repair notes:

```text
C:\tmp\ps-lx10-xv-j550-capture
```
