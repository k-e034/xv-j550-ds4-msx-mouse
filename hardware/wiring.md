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

## XV-J550 Port Measurements

Initial idle-voltage measurements taken with the breakout attached, using the
terminal identified as pin 9 for the black probe:

| Pin | Idle voltage |
| --- | --- |
| 1 | Approximately 4.95 V |
| 2 | Approximately 4.95 V |
| 3 | Approximately 4.95 V |
| 4 | Approximately 4.95 V |
| 5 | Approximately 4.95 V, confirmed using the metal chassis as ground |
| 6 | Approximately 4.95 V |
| 7 | 0 V |
| 8 | 0 V |
| 9 | Reference / 0 V |

Interpretation:

- Pins 1-4 and 6 appear to be pulled-up input/data lines.
- Pin 8 being low at idle is compatible with an MSX-style host strobe output.
- Pin 7 may be a host-controlled output or otherwise low at the measured state.
- Pin 5 is confirmed as approximately +5 V and matches the standard MSX
  general-purpose port assignment.
- With power off, pin 9 has continuity to the metal chassis.
- With power off, pin 5 causes the continuity buzzer to sound, but the measured
  resistance between pins 5 and 9 is approximately 53 ohms in one probe
  direction and OL in the reverse direction. This indicates a semiconductor
  path inside the XV-J550, not a direct short to ground.

Confirmed reference pins:

- Pin 5: +5 V
- Pin 9: Ground / metal chassis

The XIAO must still not be connected directly to any 4.95 V signal. Use the
BSS138 level converter modules between the XV-J550 signal pins and XIAO GPIO.

## Ordered Parts

Ordered parts pending receipt:

| Qty | Part No. | Item | Price |
| --- | --- | --- | --- |
| 1 | 130088 | Breadboard jumper wire set, 60+ wires, unsorted / ZYJ-W1 | 300 JPY |
| 1 | 100315 | Breadboard EIC-801 / 165-40-4-8010 | 370 JPY |
| 1 | 112031 | 0.96 inch 128x64 OLED display, white | 580 JPY |
| 1 | 117237 | D-sub 9P female breakout board VE1045 | 280 JPY |

Notes:

- The D-sub 9P female breakout is useful on the workbench side.
- Because the XV-J550 connector is recessed with little side clearance, do not assume this breakout board can plug directly into the XV-J550.
- A slim D-sub cable or compact solder-cup connector may still be needed for the XV-J550 side.

## Parts Received / Purchased

Confirmed from receipts and photos on 2026-06-06:

| Qty | Part No. / Model | Item | Intended Use |
| --- | --- | --- | --- |
| 2 | 113837 / AE-LCNV4-MOSFET(BSS138) | 4-bit bidirectional logic level converter | Eight available 3.3 V / 5 V signal channels |
| 1 | 130088 / ZYJ-W1 | Breadboard jumper wire set | Prototyping wiring |
| 1 | 100315 / EIC-801 | Breadboard | Prototype assembly |
| 1 | 112031 | 0.96 inch 128x64 white OLED | Optional status display |
| 1 | 117237 / VE1045 | D-sub 9P female breakout board | Workbench-side D-sub breakout |
| 1 | DT-10B | Pocket digital multimeter | Pin identification and voltage checks |
| 1 | goot KS-30R | 30 W soldering iron | Connector and final wiring assembly |
| 1 | goot GS-108 | Desoldering sucker | Rework and repair |
| 1 | goot SD-81 | 0.6 mm Sn60/Pb40 rosin-core solder | Fine electronics soldering |

Also pictured:

- NEC Aterm WG1800HP router; not required for the mouse interface itself.

## Remaining Fit Check

The VE1045 is a female D-sub breakout board. Before attempting connection:

1. Confirm the gender of the recessed XV-J550 mouse connector.
2. Confirm the VE1045 physically mates with it.
3. Do not force the breakout board into the recessed connector.
4. If it does not fit, obtain a slim mating DE-9 connector or cable and use the VE1045 only on the workbench side.

## Assembly Progress

- Both BSS138 4-channel logic level converter modules now have their pin
  headers soldered.
- Power-only breadboard wiring has been completed and verified:
  - XV-J550 pin 5 to both module HV rails
  - XIAO 3V3 to both module LV rails
  - XV-J550 pin 9, XIAO GND, and both module GND rails connected together
  - Both HV rails measure approximately 4.95 V
  - Both LV rails measure approximately 3.3 V
  - No abnormal voltage, heating, or smell observed

Before applying power, inspect and test both modules:

1. Visually inspect every solder joint for bridges, dull/cold joints, and loose
   pins.
2. In continuity mode, confirm GND pins on each module are connected as
   expected.
3. Confirm HV is not shorted to GND.
4. Confirm LV is not shorted to GND.
5. Confirm HV is not shorted to LV.
6. Confirm adjacent signal pins are not shorted together.

Some resistance or a brief continuity indication through the onboard MOSFETs
and pull-up resistors may occur. A steady near-zero-ohm reading is the condition
to investigate before powering the modules.

## Staged Signal Connection

Connect and verify signals in this order:

1. Strobe only:
   - XV-J550 pin 8 -> module 2 HV1
   - module 2 LV1 -> XIAO D4
2. Data lines after strobe verification:
   - XV-J550 pins 1-4 -> module 1 HV1-HV4
   - module 1 LV1-LV4 -> XIAO D0-D3
3. Buttons only after cursor movement works:
   - XV-J550 pin 6 -> module 2 HV2 -> module 2 LV2 -> XIAO D5
   - XV-J550 pin 7 -> module 2 HV3 -> module 2 LV3 -> XIAO D6

Always power off the XV-J550 and disconnect XIAO USB before changing signal
wiring.
