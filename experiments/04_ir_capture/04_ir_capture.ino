// IR capture: VS1838B OUT -> GPIO 14. Dumps every received IR frame.
// The receiver's VCC is fed from GPIO 27 driven HIGH (3.3 V): on a 400-point
// breadboard the wide 30-pin board covers the 3V3 pin's row, and the VS1838B
// only draws ~1.5 mA, well within a GPIO's 12 mA budget.
// Tuned for AC remotes (long frames), based on IRremoteESP8266's IRrecvDumpV2.
// Serial: 115200 baud. Raw timings are printed in microseconds as uint16_t arrays
// so they can be saved verbatim into captures/ and replayed later.

#include <Arduino.h>
#include <IRrecv.h>
#include <IRremoteESP8266.h>
#include <IRac.h>
#include <IRutils.h>

constexpr uint16_t kRecvPin = 14;
constexpr uint8_t kRxPowerPin = 27;
// AC frames can exceed 200 marks/spaces; leave lots of headroom.
constexpr uint16_t kCaptureBufferSize = 1024;
// Gap (ms) that ends a frame. 50 ms keeps multi-part AC messages together.
constexpr uint8_t kFrameGapMs = 50;
// Ignore short noise bursts (number of marks+spaces).
constexpr uint16_t kMinUnknownSize = 12;
constexpr uint8_t kTolerancePercent = kTolerance;

IRrecv irrecv(kRecvPin, kCaptureBufferSize, kFrameGapMs, true);
decode_results results;
uint32_t frameCount = 0;

void setup() {
  Serial.begin(115200);
  pinMode(kRxPowerPin, OUTPUT);
  digitalWrite(kRxPowerPin, HIGH);
  delay(500);  // also lets the receiver's AGC settle after power-up
  Serial.printf("\n=== IR capture on GPIO %d ===\n", kRecvPin);
  irrecv.setUnknownThreshold(kMinUnknownSize);
  irrecv.setTolerance(kTolerancePercent);
  irrecv.enableIRIn();
  Serial.println("Point a remote at the receiver and press a button.");
}

void loop() {
  if (!irrecv.decode(&results)) return;

  frameCount++;
  Serial.printf("\n########## FRAME %u  (t=%lu ms) ##########\n", frameCount,
                millis());
  if (results.overflow) {
    Serial.printf("WARNING: buffer overflow (>%d entries), capture truncated\n",
                  kCaptureBufferSize);
  }
  Serial.print(resultToHumanReadableBasic(&results));
  String ac = IRAcUtils::resultAcToString(&results);
  if (ac.length()) Serial.println("AC state: " + ac);
  Serial.println(resultToSourceCode(&results));
  yield();
}
