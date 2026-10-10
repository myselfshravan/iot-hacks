// IR transmit + receive. Replays NEC codes over an IR LED and keeps listening
// on the VS1838B, so the board can hear (and verify) its own transmissions.
//
// Wiring:
//   VS1838B  VCC <- GPIO 27 (HIGH)  GND <- GND  OUT -> GPIO 14
//   GPIO 26 -> 220R -> 2N2222A base; emitter -> GND; collector -> IR LED
//   cathode; IR LED anode -> 220R -> VIN (5 V from USB)
//
// Serial commands (115200 baud, newline-terminated):
//   nec <hex>     send a 32-bit NEC code, e.g. "nec 0xC03F50AF"
//   test <hex>    send it, then report whether our own receiver decoded it
//   led           drive the LED continuously for 3 s (phone-camera check)

#include <Arduino.h>
#include <IRrecv.h>
#include <IRremoteESP8266.h>
#include <IRsend.h>
#include <IRutils.h>

constexpr uint16_t kRecvPin = 14;
constexpr uint8_t kRxPowerPin = 27;
constexpr uint16_t kSendPin = 26;
constexpr uint16_t kCaptureBufferSize = 1024;
constexpr uint8_t kFrameGapMs = 50;
constexpr uint32_t kLoopbackWaitMs = 300;

IRrecv irrecv(kRecvPin, kCaptureBufferSize, kFrameGapMs, true);
IRsend irsend(kSendPin);
decode_results results;

void printFrame() {
  Serial.printf("RX %s 0x%s (%d bits)\n",
                typeToString(results.decode_type, results.repeat).c_str(),
                uint64ToString(results.value, 16).c_str(), results.bits);
}

void sendNec(uint32_t code) {
  irsend.sendNEC(code, 32);
  Serial.printf("TX NEC 0x%08X\n", code);
}

void loopbackTest(uint32_t code) {
  while (irrecv.decode(&results)) {}  // drop anything stale
  sendNec(code);
  uint32_t start = millis();
  while (millis() - start < kLoopbackWaitMs) {
    if (irrecv.decode(&results)) {
      bool ok = results.decode_type == NEC && results.value == code;
      printFrame();
      Serial.println(ok ? "LOOPBACK PASS" : "LOOPBACK MISMATCH");
      return;
    }
  }
  Serial.println("LOOPBACK FAIL: nothing received");
}

void ledCheck() {
  Serial.println("LED on (38 kHz carrier) for 3 s");
  irsend.enableIROut(38);
  for (int i = 0; i < 3000; i++) irsend.mark(1000);  // mark() takes uint16 us
  irsend.space(0);
  Serial.println("LED off");
}

void handleCommand(String line) {
  line.trim();
  int sp = line.indexOf(' ');
  String cmd = sp < 0 ? line : line.substring(0, sp);
  String arg = sp < 0 ? "" : line.substring(sp + 1);
  uint32_t code = strtoul(arg.c_str(), nullptr, 16);

  if (cmd == "nec" && arg.length()) {
    sendNec(code);
  } else if (cmd == "test" && arg.length()) {
    loopbackTest(code);
  } else if (cmd == "led") {
    ledCheck();
  } else if (line.length()) {
    Serial.println("ERR commands: nec <hex> | test <hex> | led");
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(kRxPowerPin, OUTPUT);
  digitalWrite(kRxPowerPin, HIGH);
  irsend.begin();
  delay(500);
  irrecv.enableIRIn();
  Serial.printf("\n=== IR TX on GPIO %d, RX on GPIO %d ===\n", kSendPin,
                kRecvPin);
  Serial.println("READY");
}

void loop() {
  if (Serial.available()) handleCommand(Serial.readStringUntil('\n'));
  if (irrecv.decode(&results)) printFrame();
}
