# Hardware Wiring Notes

![An example of wiring](others/IMG_7949.png)

## Verified Protocol

The XV-J550 mouse port has been verified to use an MSX-compatible mouse
protocol and pinout.

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

## XIAO Pin Mapping

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

## Prototype Parts

- Two 4-channel BSS138 bidirectional logic-level converter modules
- Breadboard and jumper wires
- Female DE-9 breakout board or a compact connector that fits the recessed port
- Multimeter
- Seeed Studio XIAO SAMD21

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

## Initial Integrated Test Results

- Xperia/Termux can communicate with the XIAO over USB CDC.
- With the XV-J550 powered off, the diagnostic firmware reports:

```text
pendingX=0 pendingY=0 buttons=0 nibbleIndex=0 strobe=0 edges=0
```

- With the XV-J550 powered on and the strobe/data lines connected, USB CDC
  replies can be delayed or omitted while the XV-J550 is actively polling the
  mouse port.
- After correcting the breadboard wiring, the firmware reported at least 52
  strobe edges from the XV-J550.
- The Termux USB helper successfully sent a click pulse and X-axis movement.
- The XV-J550 cursor visibly moved. This confirms the data lines, strobe line,
  level conversion, firmware nibble output, and USB command path are working
  end to end.

Known behavior:

- The powered XV-J550 polls the mouse port frequently.
- Frequent mouse-port polling can delay USB CDC diagnostic replies, but mouse
  movement commands are still processed.
