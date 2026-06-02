/*
  XV-J550 DS4 MSX Mouse Emulator

  Board: Seeed Studio XIAO SAMD21

  This firmware receives movement commands over USB serial and exposes them as
  MSX-compatible mouse nibbles on GPIO.

  Important electrical assumption:
  - Data/button outputs are open-drain style: LOW = asserted 0, INPUT/Hi-Z = 1.
  - Do not connect XIAO SAMD21 pins directly to a 5 V mouse port until the
    level shifting / open-collector interface is built.
*/

struct MouseState {
  int16_t pendingX;
  int16_t pendingY;
  uint8_t buttons;
};

MouseState state = {0, 0, 0};

// XIAO SAMD21 Arduino pin numbers. These are provisional and should be mapped
// through an external level/interface board before the XV-J550 is connected.
const uint8_t PIN_DATA0 = 0;     // MSX port pin 1, bit 0
const uint8_t PIN_DATA1 = 1;     // MSX port pin 2, bit 1
const uint8_t PIN_DATA2 = 2;     // MSX port pin 3, bit 2
const uint8_t PIN_DATA3 = 3;     // MSX port pin 4, bit 3
const uint8_t PIN_STROBE = 4;    // MSX port pin 8, host clock/control
const uint8_t PIN_BUTTON_L = 5;  // MSX port pin 6, active low
const uint8_t PIN_BUTTON_R = 6;  // MSX port pin 7, active low

const uint8_t DATA_PINS[4] = {PIN_DATA0, PIN_DATA1, PIN_DATA2, PIN_DATA3};
const uint32_t FRAME_RESET_US = 3000;

uint8_t packetNibbles[4] = {0, 0, 0, 0};
uint8_t nibbleIndex = 0;
int lastStrobe = LOW;
uint32_t lastClockUs = 0;

int8_t takeClampedDelta(int16_t &value) {
  int16_t out = constrain(value, -128, 127);
  value -= out;
  return (int8_t)out;
}

void releasePin(uint8_t pin) {
  pinMode(pin, INPUT);
}

void pullLow(uint8_t pin) {
  digitalWrite(pin, LOW);
  pinMode(pin, OUTPUT);
}

void writeOpenDrainBit(uint8_t pin, bool valueIsOne) {
  if (valueIsOne) {
    releasePin(pin);
  } else {
    pullLow(pin);
  }
}

void writeNibble(uint8_t nibble) {
  for (uint8_t bit = 0; bit < 4; bit++) {
    writeOpenDrainBit(DATA_PINS[bit], (nibble & (1 << bit)) != 0);
  }
}

void writeButtons(uint8_t buttons) {
  writeOpenDrainBit(PIN_BUTTON_L, (buttons & 0x01) == 0);
  writeOpenDrainBit(PIN_BUTTON_R, (buttons & 0x02) == 0);
}

void latchPacket() {
  noInterrupts();
  int8_t x = takeClampedDelta(state.pendingX);
  int8_t y = takeClampedDelta(state.pendingY);
  uint8_t buttons = state.buttons;
  interrupts();

  // MSX convention: positive X means left, positive Y means up.
  uint8_t ux = (uint8_t)x;
  uint8_t uy = (uint8_t)y;

  packetNibbles[0] = (ux >> 4) & 0x0F;
  packetNibbles[1] = ux & 0x0F;
  packetNibbles[2] = (uy >> 4) & 0x0F;
  packetNibbles[3] = uy & 0x0F;
  nibbleIndex = 0;
  writeNibble(packetNibbles[nibbleIndex]);
  writeButtons(buttons);
}

void setupPins() {
  for (uint8_t i = 0; i < 4; i++) {
    releasePin(DATA_PINS[i]);
  }
  pinMode(PIN_STROBE, INPUT);
  releasePin(PIN_BUTTON_L);
  releasePin(PIN_BUTTON_R);
  lastStrobe = digitalRead(PIN_STROBE);
  latchPacket();
}

void printHelp() {
  Serial.println("Commands:");
  Serial.println("  M <dx> <dy> <buttons>  add movement and set buttons");
  Serial.println("  Z                      clear pending movement");
  Serial.println("  S                      print status");
  Serial.println("Buttons: bit0=left, bit1=right");
}

void setup() {
  setupPins();
  Serial.begin(115200);
  uint32_t start = millis();
  while (!Serial && millis() - start < 1500) {
    delay(10);
  }
  Serial.println("XV-J550 MSX mouse emulator");
  printHelp();
}

void addMovement(int dx, int dy, uint8_t buttons) {
  noInterrupts();
  state.pendingX = constrain((int32_t)state.pendingX + dx, -1024, 1024);
  state.pendingY = constrain((int32_t)state.pendingY + dy, -1024, 1024);
  state.buttons = buttons;
  interrupts();
  writeButtons(buttons);
}

void printStatus() {
  noInterrupts();
  int16_t x = state.pendingX;
  int16_t y = state.pendingY;
  uint8_t buttons = state.buttons;
  interrupts();

  Serial.print("pendingX=");
  Serial.print(x);
  Serial.print(" pendingY=");
  Serial.print(y);
  Serial.print(" buttons=");
  Serial.print(buttons);
  Serial.print(" nibbleIndex=");
  Serial.print(nibbleIndex);
  Serial.print(" strobe=");
  Serial.println(digitalRead(PIN_STROBE));
}

void handleSerialLine(String line) {
  line.trim();
  if (line.length() == 0) {
    return;
  }

  if (line == "S") {
    printStatus();
    return;
  }

  if (line == "Z") {
    noInterrupts();
    state.pendingX = 0;
    state.pendingY = 0;
    interrupts();
    latchPacket();
    Serial.println("OK zeroed");
    return;
  }

  int dx = 0;
  int dy = 0;
  int buttons = 0;

  if (sscanf(line.c_str(), "M %d %d %d", &dx, &dy, &buttons) == 3) {
    addMovement(dx, dy, (uint8_t)(buttons & 0x03));
    Serial.println("OK");
  } else {
    Serial.println("ERR expected: M <dx> <dy> <buttons>, Z, or S");
  }
}

void pollSerial() {
  static String line;

  while (Serial.available() > 0) {
    char c = (char)Serial.read();
    if (c == '\n') {
      handleSerialLine(line);
      line = "";
    } else if (c != '\r' && line.length() < 80) {
      line += c;
    }
  }
}

void pollMouseHost() {
  uint32_t now = micros();
  if (now - lastClockUs > FRAME_RESET_US) {
    if (nibbleIndex != 0) {
      latchPacket();
    }
  }

  int strobe = digitalRead(PIN_STROBE);
  if (strobe != lastStrobe) {
    lastStrobe = strobe;
    lastClockUs = now;
    nibbleIndex = (nibbleIndex + 1) & 0x03;
    writeNibble(packetNibbles[nibbleIndex]);
  }
}

void loop() {
  pollSerial();
  pollMouseHost();
}
