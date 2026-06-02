# MSX Mouse Protocol Notes

## Working Model

An MSX mouse reports movement as signed X/Y deltas. The host toggles a control line, and the mouse returns movement data in 4-bit nibbles over the data pins.

This project should emulate the mouse side.

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

