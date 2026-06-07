# Termux USB Test

The Xperia can act as an SSH-accessible USB host for the XIAO SAMD21.

## Prerequisites

- Termux and Termux:API installed from the same source
- `termux-api` package installed
- XIAO connected to the Xperia through USB OTG
- USB access permission granted

Confirm the USB device:

```bash
termux-usb -l
termux-usb -r /dev/bus/usb/001/002
```

The device path may change after reconnecting it.

## Build

```bash
pkg install clang
cd ~/xv-j550-ds4-msx-mouse/android/termux
clang -O2 -Wall -Wextra -o xiao_usb_cdc xiao_usb_cdc.c
```

## Run

```bash
termux-usb -e ./xiao_usb_cdc /dev/bus/usb/001/002
```

The helper sends this conservative smoke-test sequence:

```text
S
M 0 0 1
M 0 0 0
M 30 0 0
S
Z
S
```

This produces a roughly 300 ms left-click pulse before testing horizontal
movement. The click may exit the XV-J550 demo mode.

## Custom SSH Test Commands

Create `~/.xiao_mouse_commands` on the Xperia to replace the built-in test
sequence:

```bash
cat > ~/.xiao_mouse_commands <<'EOF'
M 50 0 0
M -50 0 0
M 0 50 0
M 0 -50 0
M 0 0 1
M 0 0 0
S
EOF
```

Then run:

```bash
termux-usb -e "$PWD/xiao_usb_cdc" /dev/bus/usb/001/002
```

Delete the file to restore the built-in smoke test:

```bash
rm ~/.xiao_mouse_commands
```

Expected replies include `pendingX=...`, `OK`, and `OK zeroed`.

The USB CDC interface and endpoint numbers are currently based on the standard
Seeed/Arduino SAMD USB CDC layout:

```text
control interface: 0
data interface:    1
bulk OUT endpoint: 0x02
bulk IN endpoint:  0x83
```

If interface claiming or bulk transfers fail, inspect the XIAO USB descriptors
before changing the hardware wiring.
