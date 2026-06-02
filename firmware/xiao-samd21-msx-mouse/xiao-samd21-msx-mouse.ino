/*
  XV-J550 DS4 MSX Mouse Emulator

  Board: Seeed Studio XIAO SAMD21

  This sketch is an early scaffold. It receives movement commands over USB
  serial and will later expose them as MSX-compatible mouse nibbles on GPIO.
*/

struct MouseState {
  int16_t dx;
  int16_t dy;
  uint8_t buttons;
};

MouseState state = {0, 0, 0};

// TODO: Replace these placeholders after confirming the XV-J550 pinout.
const uint8_t PIN_DATA0 = 0;
const uint8_t PIN_DATA1 = 1;
const uint8_t PIN_DATA2 = 2;
const uint8_t PIN_DATA3 = 3;
const uint8_t PIN_STROBE = 4;
const uint8_t PIN_BUTTON_L = 5;
const uint8_t PIN_BUTTON_R = 6;

void setupPins() {
  pinMode(PIN_DATA0, INPUT);
  pinMode(PIN_DATA1, INPUT);
  pinMode(PIN_DATA2, INPUT);
  pinMode(PIN_DATA3, INPUT);
  pinMode(PIN_STROBE, INPUT);
  pinMode(PIN_BUTTON_L, INPUT);
  pinMode(PIN_BUTTON_R, INPUT);
}

void setup() {
  setupPins();
  Serial.begin(115200);
  while (!Serial) {
    delay(10);
  }
  Serial.println("XV-J550 MSX mouse emulator scaffold");
  Serial.println("Command: M <dx> <dy> <buttons>");
}

void addMovement(int dx, int dy, uint8_t buttons) {
  state.dx = constrain((int32_t)state.dx + dx, -127, 127);
  state.dy = constrain((int32_t)state.dy + dy, -127, 127);
  state.buttons = buttons;
}

void handleSerialLine(String line) {
  line.trim();
  if (line.length() == 0) {
    return;
  }

  int dx = 0;
  int dy = 0;
  int buttons = 0;

  if (sscanf(line.c_str(), "M %d %d %d", &dx, &dy, &buttons) == 3) {
    addMovement(dx, dy, (uint8_t)(buttons & 0x03));
    Serial.print("OK dx=");
    Serial.print(state.dx);
    Serial.print(" dy=");
    Serial.print(state.dy);
    Serial.print(" buttons=");
    Serial.println(state.buttons);
  } else {
    Serial.println("ERR expected: M <dx> <dy> <buttons>");
  }
}

void pollSerial() {
  static String line;

  while (Serial.available() > 0) {
    char c = (char)Serial.read();
    if (c == '\n') {
      handleSerialLine(line);
      line = "";
    } else if (c != '\r') {
      line += c;
    }
  }
}

void loop() {
  pollSerial();

  // TODO: Watch host strobe and output MSX mouse nibbles.
}

