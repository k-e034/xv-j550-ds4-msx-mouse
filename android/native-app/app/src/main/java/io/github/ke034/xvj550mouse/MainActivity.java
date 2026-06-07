package io.github.ke034.xvj550mouse;

import android.app.Activity;
import android.app.PendingIntent;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.graphics.Color;
import android.hardware.input.InputManager;
import android.hardware.usb.UsbConstants;
import android.hardware.usb.UsbDevice;
import android.hardware.usb.UsbDeviceConnection;
import android.hardware.usb.UsbEndpoint;
import android.hardware.usb.UsbInterface;
import android.hardware.usb.UsbManager;
import android.os.Build;
import android.os.Bundle;
import android.os.Handler;
import android.os.Looper;
import android.view.Gravity;
import android.view.InputDevice;
import android.view.KeyEvent;
import android.view.MotionEvent;
import android.view.View;
import android.view.WindowManager;
import android.widget.Button;
import android.widget.LinearLayout;
import android.widget.SeekBar;
import android.widget.TextView;

import java.nio.charset.StandardCharsets;
import java.util.Locale;

public class MainActivity extends Activity implements InputManager.InputDeviceListener {
    private static final String USB_PERMISSION = "io.github.ke034.xvj550mouse.USB_PERMISSION";
    private static final int XIAO_VENDOR_ID = 0x2886;
    private static final int XIAO_PRODUCT_ID = 0x802F;
    private static final int FRAME_MS = 16;
    private static final float DEAD_ZONE = 0.18f;

    private final Handler handler = new Handler(Looper.getMainLooper());
    private UsbManager usbManager;
    private UsbDeviceConnection usbConnection;
    private UsbInterface controlInterface;
    private UsbInterface dataInterface;
    private UsbEndpoint bulkOut;

    private TextView connectionText;
    private TextView controllerText;
    private TextView inputText;
    private TextView mappingText;
    private SeekBar speedBar;

    private float axisX;
    private float axisY;
    private double remainderX;
    private double remainderY;
    private int buttons;
    private int lastSentButtons = -1;

    private final Runnable sendFrame = new Runnable() {
        @Override
        public void run() {
            sendCurrentInput();
            handler.postDelayed(this, FRAME_MS);
        }
    };

    private final BroadcastReceiver usbReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            if (USB_PERMISSION.equals(intent.getAction())) {
                UsbDevice device = intent.getParcelableExtra(UsbManager.EXTRA_DEVICE);
                if (intent.getBooleanExtra(UsbManager.EXTRA_PERMISSION_GRANTED, false) && device != null) {
                    openXiao(device);
                } else {
                    setConnectionStatus("USB permission denied", Color.RED);
                }
            } else if (UsbManager.ACTION_USB_DEVICE_DETACHED.equals(intent.getAction())) {
                closeUsb();
            }
        }
    };

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        getWindow().addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);
        usbManager = (UsbManager) getSystemService(USB_SERVICE);
        buildUi();

        IntentFilter filter = new IntentFilter();
        filter.addAction(USB_PERMISSION);
        filter.addAction(UsbManager.ACTION_USB_DEVICE_DETACHED);
        if (Build.VERSION.SDK_INT >= 33) {
            registerReceiver(usbReceiver, filter, Context.RECEIVER_NOT_EXPORTED);
        } else {
            registerReceiver(usbReceiver, filter);
        }

        InputManager inputManager = (InputManager) getSystemService(INPUT_SERVICE);
        inputManager.registerInputDeviceListener(this, null);
        updateControllerStatus();
        requestXiaoConnection();
        handler.post(sendFrame);
    }

    private void buildUi() {
        int pad = dp(18);
        LinearLayout root = new LinearLayout(this);
        root.setOrientation(LinearLayout.VERTICAL);
        root.setPadding(pad, pad, pad, pad);
        root.setGravity(Gravity.CENTER_HORIZONTAL);

        TextView title = text("XV-J550 Mouse", 26, Color.BLACK);
        title.setGravity(Gravity.CENTER);
        root.addView(title, matchWrap());

        connectionText = text("XIAO: disconnected", 18, Color.RED);
        controllerText = text("Controller: not detected", 18, Color.DKGRAY);
        inputText = text("X 0.00   Y 0.00   buttons 0", 20, Color.BLACK);
        inputText.setGravity(Gravity.CENTER);
        mappingText = text("Left stick: move\nCross / R1: left click\nCircle / L1: right click", 16, Color.DKGRAY);

        root.addView(connectionText, spaced());
        root.addView(controllerText, spaced());
        root.addView(inputText, spaced());

        TextView speedLabel = text("Speed", 16, Color.BLACK);
        root.addView(speedLabel, spaced());
        speedBar = new SeekBar(this);
        speedBar.setMax(24);
        speedBar.setProgress(8);
        root.addView(speedBar, matchWrap());

        Button connect = new Button(this);
        connect.setText("Connect XIAO");
        connect.setOnClickListener(v -> requestXiaoConnection());
        root.addView(connect, spaced());

        Button center = new Button(this);
        center.setText("Stop movement");
        center.setOnClickListener(v -> {
            axisX = 0;
            axisY = 0;
            remainderX = 0;
            remainderY = 0;
            sendCommand(0, 0, buttons);
            updateInputText();
        });
        root.addView(center, spaced());
        root.addView(mappingText, spaced());

        setContentView(root);
    }

    private TextView text(String value, int sp, int color) {
        TextView view = new TextView(this);
        view.setText(value);
        view.setTextSize(sp);
        view.setTextColor(color);
        return view;
    }

    private LinearLayout.LayoutParams matchWrap() {
        return new LinearLayout.LayoutParams(LinearLayout.LayoutParams.MATCH_PARENT,
                LinearLayout.LayoutParams.WRAP_CONTENT);
    }

    private LinearLayout.LayoutParams spaced() {
        LinearLayout.LayoutParams params = matchWrap();
        params.topMargin = dp(16);
        return params;
    }

    private int dp(int value) {
        return Math.round(value * getResources().getDisplayMetrics().density);
    }

    private void requestXiaoConnection() {
        for (UsbDevice device : usbManager.getDeviceList().values()) {
            if (device.getVendorId() == XIAO_VENDOR_ID && device.getProductId() == XIAO_PRODUCT_ID) {
                if (usbManager.hasPermission(device)) {
                    openXiao(device);
                } else {
                    Intent permission = new Intent(USB_PERMISSION).setPackage(getPackageName());
                    PendingIntent permissionIntent = PendingIntent.getBroadcast(
                            this, 0, permission, PendingIntent.FLAG_IMMUTABLE);
                    usbManager.requestPermission(device, permissionIntent);
                }
                return;
            }
        }
        setConnectionStatus("XIAO: not found", Color.RED);
    }

    private void openXiao(UsbDevice device) {
        closeUsb();
        controlInterface = null;
        dataInterface = null;
        bulkOut = null;

        for (int i = 0; i < device.getInterfaceCount(); i++) {
            UsbInterface usbInterface = device.getInterface(i);
            if (usbInterface.getInterfaceClass() == UsbConstants.USB_CLASS_COMM) {
                controlInterface = usbInterface;
            } else if (usbInterface.getInterfaceClass() == UsbConstants.USB_CLASS_CDC_DATA) {
                dataInterface = usbInterface;
                for (int e = 0; e < usbInterface.getEndpointCount(); e++) {
                    UsbEndpoint endpoint = usbInterface.getEndpoint(e);
                    if (endpoint.getType() == UsbConstants.USB_ENDPOINT_XFER_BULK
                            && endpoint.getDirection() == UsbConstants.USB_DIR_OUT) {
                        bulkOut = endpoint;
                    }
                }
            }
        }

        if (controlInterface == null || dataInterface == null || bulkOut == null) {
            setConnectionStatus("XIAO: CDC interfaces not found", Color.RED);
            return;
        }

        usbConnection = usbManager.openDevice(device);
        if (usbConnection == null
                || !usbConnection.claimInterface(controlInterface, true)
                || !usbConnection.claimInterface(dataInterface, true)) {
            closeUsb();
            setConnectionStatus("XIAO: failed to open USB", Color.RED);
            return;
        }

        byte[] lineCoding = new byte[]{0x00, (byte) 0xC2, 0x01, 0x00, 0x00, 0x00, 0x08};
        usbConnection.controlTransfer(0x21, 0x20, 0, controlInterface.getId(), lineCoding,
                lineCoding.length, 2000);
        usbConnection.controlTransfer(0x21, 0x22, 3, controlInterface.getId(), null, 0, 2000);
        setConnectionStatus("XIAO: connected", Color.rgb(0, 120, 60));
    }

    private void closeUsb() {
        if (usbConnection != null) {
            if (controlInterface != null) usbConnection.releaseInterface(controlInterface);
            if (dataInterface != null) usbConnection.releaseInterface(dataInterface);
            usbConnection.close();
        }
        usbConnection = null;
        bulkOut = null;
        lastSentButtons = -1;
        if (connectionText != null) setConnectionStatus("XIAO: disconnected", Color.RED);
    }

    private void setConnectionStatus(String status, int color) {
        connectionText.setText(status);
        connectionText.setTextColor(color);
    }

    @Override
    public boolean dispatchGenericMotionEvent(MotionEvent event) {
        if ((event.getSource() & InputDevice.SOURCE_JOYSTICK) == InputDevice.SOURCE_JOYSTICK
                && event.getAction() == MotionEvent.ACTION_MOVE) {
            showActiveController(event.getDevice());
            axisX = centeredAxis(event, MotionEvent.AXIS_X);
            axisY = centeredAxis(event, MotionEvent.AXIS_Y);
            updateInputText();
            return true;
        }
        return super.dispatchGenericMotionEvent(event);
    }

    private float centeredAxis(MotionEvent event, int axis) {
        InputDevice.MotionRange range = event.getDevice().getMotionRange(axis, event.getSource());
        if (range == null) return 0;
        float value = event.getAxisValue(axis);
        float flat = Math.max(DEAD_ZONE, range.getFlat());
        if (Math.abs(value) <= flat) return 0;
        return Math.signum(value) * (Math.abs(value) - flat) / (1.0f - flat);
    }

    @Override
    public boolean dispatchKeyEvent(KeyEvent event) {
        int mask = buttonMask(event.getKeyCode());
        if (mask != 0) {
            showActiveController(event.getDevice());
            if (event.getAction() == KeyEvent.ACTION_DOWN) buttons |= mask;
            if (event.getAction() == KeyEvent.ACTION_UP) buttons &= ~mask;
            updateInputText();
            sendCommand(0, 0, buttons);
            return true;
        }
        return super.dispatchKeyEvent(event);
    }

    private int buttonMask(int keyCode) {
        if (keyCode == KeyEvent.KEYCODE_BUTTON_A || keyCode == KeyEvent.KEYCODE_BUTTON_R1) return 1;
        if (keyCode == KeyEvent.KEYCODE_BUTTON_B || keyCode == KeyEvent.KEYCODE_BUTTON_L1) return 2;
        return 0;
    }

    private void sendCurrentInput() {
        double speed = speedBar.getProgress() + 1;
        remainderX += -axisX * speed;
        remainderY += -axisY * speed;
        int dx = (int) remainderX;
        int dy = (int) remainderY;
        remainderX -= dx;
        remainderY -= dy;

        if (dx != 0 || dy != 0 || buttons != lastSentButtons) {
            sendCommand(dx, dy, buttons);
            lastSentButtons = buttons;
        }
    }

    private void sendCommand(int dx, int dy, int buttonBits) {
        if (usbConnection == null || bulkOut == null) return;
        byte[] command = String.format(Locale.US, "M %d %d %d\n", dx, dy, buttonBits)
                .getBytes(StandardCharsets.US_ASCII);
        usbConnection.bulkTransfer(bulkOut, command, command.length, 100);
    }

    private void updateInputText() {
        inputText.setText(String.format(Locale.US, "X %.2f   Y %.2f   buttons %d", axisX, axisY, buttons));
    }

    private void updateControllerStatus() {
        String name = "not detected";
        int[] ids = InputDevice.getDeviceIds();
        for (int id : ids) {
            InputDevice device = InputDevice.getDevice(id);
            if (device != null && ((device.getSources() & InputDevice.SOURCE_GAMEPAD) != 0
                    || (device.getSources() & InputDevice.SOURCE_JOYSTICK) != 0)) {
                name = device.getName();
                break;
            }
        }
        controllerText.setText("Controller: " + name);
    }

    private void showActiveController(InputDevice device) {
        String name = device == null ? "Virtual" : device.getName();
        controllerText.setText("Controller: " + name + " (active)");
        controllerText.setTextColor(Color.rgb(0, 120, 60));
    }

    @Override
    public void onInputDeviceAdded(int deviceId) {
        updateControllerStatus();
    }

    @Override
    public void onInputDeviceRemoved(int deviceId) {
        updateControllerStatus();
    }

    @Override
    public void onInputDeviceChanged(int deviceId) {
        updateControllerStatus();
    }

    @Override
    protected void onDestroy() {
        handler.removeCallbacks(sendFrame);
        ((InputManager) getSystemService(INPUT_SERVICE)).unregisterInputDeviceListener(this);
        unregisterReceiver(usbReceiver);
        closeUsb();
        super.onDestroy();
    }
}
