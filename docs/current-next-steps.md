# Current Next Steps

These steps can be done with the hardware already available:

## 1. Flash XIAO Firmware

Open `firmware/xiao-samd21-msx-mouse/xiao-samd21-msx-mouse.ino` in the Arduino IDE.

Board target:

```text
Seeed Studio XIAO SAMD21
```

Upload the sketch and open the serial monitor at:

```text
115200 baud
Newline enabled
```

Try:

```text
S
M 5 0 0
S
M 0 0 1
S
Z
S
```

This repository was also tested with Arduino CLI 1.5.0 and Seeed SAMD Boards 1.8.6:

```text
tools\arduino-cli\arduino-cli.exe config add board_manager.additional_urls https://files.seeedstudio.com/arduino/package_seeeduino_boards_index.json
tools\arduino-cli\arduino-cli.exe core update-index
tools\arduino-cli\arduino-cli.exe core install Seeeduino:samd
tools\arduino-cli\arduino-cli.exe compile --fqbn Seeeduino:samd:seeed_XIAO_m0 firmware\xiao-samd21-msx-mouse
tools\arduino-cli\arduino-cli.exe upload -p COM3 --fqbn Seeeduino:samd:seeed_XIAO_m0 firmware\xiao-samd21-msx-mouse
```

The connected XIAO was detected as:

```text
COM3 USB Serial Device VID:PID=2886:802F
```

After upload, the firmware replied to:

```text
S
M 5 0 0
S
Z
S
```

## 2. Confirm Android USB Serial

Before DS4 input, prove that Android can send commands to the XIAO.

On this Windows development machine, use the local Conda environment first:

```text
C:\tmp\xv-j550-ds4-msx-mouse\.conda-env\python.exe android\termux\ds4_to_xiao.py --list-ports
```

After the XIAO appears as a COM port:

```text
C:\tmp\xv-j550-ds4-msx-mouse\.conda-env\python.exe android\termux\ds4_to_xiao.py --port COM3 --repeat 5
```

Replace `COM3` with the detected port.

In Termux, the final command will look like:

```text
python ds4_to_xiao.py --port /dev/ttyACM0 --repeat 5
```

If Android does not expose `/dev/ttyACM0`, USB serial may require an Android app or a Termux API workaround.

The Xperia/Termux USB host path is now available through Termux:API:

```text
termux-usb -l
termux-usb -r /dev/bus/usb/001/002
cd ~/xv-j550-ds4-msx-mouse/android/termux
clang -O2 -Wall -Wextra -o xiao_usb_cdc xiao_usb_cdc.c
termux-usb -e ./xiao_usb_cdc /dev/bus/usb/001/002
```

## 3. Measure XV-J550 Mouse Port

Do not connect the XIAO yet.

Use a multimeter and record:

- Pin 5 to pin 9 voltage
- Idle voltage of pins 1-4 to pin 9
- Idle voltage of pins 6-8 to pin 9

## 4. Build Level Interface

The firmware assumes open-drain behavior:

```text
0 = pull line low
1 = release line
```

The external circuit must protect the 3.3 V XIAO from 5 V input and pull-up levels.

## Verified Milestone

End-to-end cursor movement is working:

```text
Xperia Termux USB helper
  -> XIAO USB CDC
  -> MSX mouse nibble output
  -> BSS138 level conversion
  -> XV-J550
```

Next development steps:

1. Verify left-click and release behavior independently.
2. Use `~/.xiao_mouse_commands` for directional movement tests.
3. Connect and verify right-click on XV-J550 pin 7.
4. Replace command-file tests with continuous movement commands.
5. Capture DUALSHOCK 4 input using an Android input API.
6. Add dead-zone, speed scaling, axis inversion, and button mapping.

Directional testing found and corrected a one-nibble strobe phase error.
The corrected firmware has been verified:

```text
M 1 0 0   -> left
M -1 0 0  -> right
M 0 1 0   -> up
M 0 -1 0  -> down
```

Left-button down and release are also verified.

Right-click is wired to XV-J550 pin 7 through XIAO D6. Re-test it on the layout
screen, because the XV-J550 manual indicates that the creation screen does not
use right-click.
