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

TODO: Confirm the XV-J550 mouse port pinout from the service manual or by continuity/voltage inspection before connecting the XIAO.

Standard MSX general purpose port pinout, computer-side numbering:

| Pin | Direction from computer | Role |
| --- | --- | --- |
| 1 | Input | Data bit 0 / joystick up |
| 2 | Input | Data bit 1 / joystick down |
| 3 | Input | Data bit 2 / joystick left |
| 4 | Input | Data bit 3 / joystick right |
| 5 | Power | +5 V, typically limited current |
| 6 | Input/Output | Trigger A / left button |
| 7 | Input/Output | Trigger B / right button |
| 8 | Output | Mouse strobe/clock from host |
| 9 | Ground | Signal ground |

## Provisional XIAO Pin Mapping

Do not treat this as a direct wiring table. Put the level/interface circuit between these pins and the XV-J550.

| XIAO pin | MSX port pin | Firmware name |
| --- | --- | --- |
| D0 | 1 | DATA0 |
| D1 | 2 | DATA1 |
| D2 | 3 | DATA2 |
| D3 | 4 | DATA3 |
| D4 | 8 | STROBE input |
| D5 | 6 | Left button |
| D6 | 7 | Right button |
| GND | 9 | Ground |

## Pre-Connection Checklist

1. With the XV-J550 powered off, identify pin 9 ground on the mouse connector.
2. With the XV-J550 powered on and no adapter connected, measure pin 5 against pin 9. Expect around +5 V.
3. Measure pins 1-4, 6, 7, and 8 against pin 9. Record idle voltages.
4. Do not connect pin 8 to the XIAO until it is confirmed not to exceed 3.3 V at the XIAO input after level shifting.
5. Confirm that XIAO data/button outputs can only pull the target line low or release it.
