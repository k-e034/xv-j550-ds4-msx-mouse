# Hardware Wiring Notes

## Current Assumption

The XV-J550 mouse port is assumed to be MSX mouse compatible.

MSX-style mouse ports use a 9-pin connector, but they are not RS-232C serial ports. The signal lines are GPIO-style control and data lines.

## Required Electrical Interface

The Seeed Studio XIAO SAMD21 is a 3.3 V microcontroller. The target mouse port may use 5 V pull-ups and 5 V logic.

Use one of the following approaches before connecting directly:

- Open-collector / open-drain buffer
- Transistor interface
- 74LS05 / 74LS06 / similar open-collector inverter buffer, with logic adjusted in firmware
- MOSFET level shifter where appropriate
- Resistor divider for input-only lines

## Do Not Connect

Do not connect an RS-232C adapter to the XV-J550 mouse port.

Do not feed 5 V logic directly into XIAO SAMD21 GPIO pins.

## Pinout

TODO: Confirm the XV-J550 mouse port pinout from the service manual or by continuity/voltage inspection.

Expected MSX-style signal categories:

- 4 data lines for nibble output
- Strobe/control line from the host
- Button lines
- +5 V supply
- Ground

