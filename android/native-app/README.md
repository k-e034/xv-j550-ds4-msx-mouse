# Native Android DS4 Bridge

This app reads a Bluetooth-connected DUALSHOCK 4 using Android's standard
game-controller APIs and writes mouse commands directly to the XIAO SAMD21 over
USB CDC.

## Current Mapping

```text
DS4 left stick  -> cursor movement
Cross or R1     -> left click
Circle or L1    -> right click
```

## USB Device

The app filters for the connected XIAO SAMD21:

```text
VID:PID = 2886:802F
```

Android will show a USB permission dialog when the XIAO is connected.

## Build

Open `android/native-app` in Android Studio and build/install the `app` module.

Requirements:

- Android Studio with Android SDK 34
- JDK 17
- Xperia running Android 10 or later

GitHub Actions also builds a debug APK when files under `android/native-app`
change. Download the `xv-j550-mouse-debug-apk` workflow artifact and install
`app-debug.apk` on the Xperia.

## First Test

1. Pair the DS4 with the Xperia over Bluetooth.
2. Power the XV-J550 and connect the verified mouse interface.
3. Connect the XIAO to the Xperia through USB OTG.
4. Open `XV-J550 Mouse` and allow USB access.
5. Confirm both `XIAO: connected` and the controller name are displayed.
6. Move the left stick slowly and test Cross/Circle.

The speed slider controls movement per 16 ms frame. Start at a low value.

Close any active `termux-usb -e ...` command before opening the app so only one
process owns the XIAO USB interface.

## Verified Xperia Behavior

The app successfully transmits Bluetooth DUALSHOCK 4 input to the XV-J550.

On the tested Xperia, the controller name is reported by Android as `Virtual`
rather than `Wireless Controller`. This does not prevent operation. Once an
input event is received, the app marks the controller status as active.
