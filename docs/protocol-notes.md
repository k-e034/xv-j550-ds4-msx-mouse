# MSX Mouse Protocol Notes

## Working Model

An MSX mouse reports movement as signed X/Y deltas. The host toggles a control line, and the mouse returns movement data in 4-bit nibbles over the data pins.

This project should emulate the mouse side.

Public MSX protocol notes describe communication as 4-bit nibbles using pin 8 as a clock. The original protocol sends the high nibble first:

```text
Nibble 0: X delta bits 7..4
Nibble 1: X delta bits 3..0
Nibble 2: Y delta bits 7..4
Nibble 3: Y delta bits 3..0
```

Signed delta convention:

- Positive X means the mouse moved left.
- Positive Y means the mouse moved up.

## Data Flow

```text
Android sends:
  dx, dy, buttons

XIAO stores:
  accumulated_dx
  accumulated_dy
  current_buttons

XV-J550 reads:
  nibbles returned on mouse data pins
```

## Unknowns

- Exact XV-J550 pinout
- Required timing tolerance
- Button polarity
- Axis direction
- Whether X/Y deltas need inversion
- Whether the host expects the standard MSX read sequence without variation

## Test Strategy

Start without DS4 input.

Use serial commands like:

```text
M 5 0 0
M -5 0 0
M 0 5 0
M 0 -5 0
M 0 0 1
```

Then confirm cursor movement and button behavior on the XV-J550.

## Sources

- MSX Wiki Mouse/Trackball: https://www.msx.org/wiki/Mouse/Trackball
- MSX Wiki General Purpose port: https://www.msx.org/wiki/General_Purpose_port
