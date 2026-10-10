// IR receiver diagnostic, no multimeter needed: the ESP32 measures the
// receiver's OUT line itself. Same wiring as 04_ir_capture:
//   VS1838B VCC <- GPIO 27 (driven HIGH)   GND <- GND   OUT -> GPIO 14
//
// Every 250 ms prints:
//   mV     - voltage on OUT (ADC). A powered VS1838B idles HIGH (~3000+ mV).
//            Near 0 or wandering => receiver unpowered / reversed / loose.
//   edges  - HIGH<->LOW transitions seen in the window. 0 at rest; jumps to
//            dozens or hundreds while a remote button is held.
//   lowUs  - total time OUT spent LOW (an IR burst pulls it LOW).

constexpr uint8_t kOutPin = 14;
constexpr uint8_t kPowerPin = 27;
constexpr uint32_t kWindowMs = 250;

void setup() {
  Serial.begin(115200);
  pinMode(kPowerPin, OUTPUT);
  digitalWrite(kPowerPin, HIGH);
  delay(500);
  Serial.println("\n=== IR receiver diagnostic (OUT=GPIO14, VCC=GPIO27) ===");
}

void loop() {
  uint32_t mv = analogReadMilliVolts(kOutPin);

  pinMode(kOutPin, INPUT);
  uint32_t edges = 0, lowUs = 0;
  int last = digitalRead(kOutPin);
  uint32_t lastChange = micros();
  uint32_t start = millis();
  while (millis() - start < kWindowMs) {
    int now = digitalRead(kOutPin);
    if (now != last) {
      uint32_t t = micros();
      if (last == LOW) lowUs += t - lastChange;
      lastChange = t;
      last = now;
      edges++;
    }
  }
  if (last == LOW) lowUs += micros() - lastChange;

  Serial.printf("mV=%4u  edges=%4u  lowUs=%6u  %s\n", mv, edges, lowUs,
                edges ? "<-- IR ACTIVITY" : "");
}
