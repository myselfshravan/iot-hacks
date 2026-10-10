// 1.8" ST7735R TFT (128x160, SPI) bring-up for Dexi, using Adafruit_ST7735.
// (LovyanGFX's ST7735/ST7735S init garbled colours on this module; Adafruit's
// INITR_BLACKTAB works, found with 12_tft_variants.)
//
// Shows a test pattern, then a blinking face drawn into an off-screen canvas
// and pushed whole, so it doesn't flicker.
//
// Wiring (TFT -> ESP32), all on the ESP32's left header (docs/wiring.md, Dexi):
//   VCC -> 3V3    GND -> GND    SCK -> D18    SDA -> D23
//   CS -> D5      A0 -> D16     RESET -> D17  LED -> D4
//
// Serial tuning: "rot <0-3>", "mhz <n>", "tab <1-6>" (variant order of 12_tft_variants).

#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>
#include <SPI.h>

constexpr int kCs = 5, kDc = 16, kRst = 17, kBacklight = 4;

Adafruit_ST7735 tft(&SPI, kCs, kDc, kRst);  // hardware SPI: SCK 18, MOSI 23
GFXcanvas16 frame(128, 160);

const uint8_t kTabs[] = {INITR_BLACKTAB, INITR_GREENTAB, INITR_REDTAB,
                         INITR_18GREENTAB, INITR_18REDTAB, INITR_18BLACKTAB};
uint8_t tab = INITR_BLACKTAB;
uint8_t rotation = 2;  // header at the top as the user holds it
uint32_t spiHz = 20000000;

void testPattern() {
  const uint16_t bars[] = {ST77XX_RED, ST77XX_GREEN, ST77XX_BLUE, ST77XX_WHITE, ST77XX_BLACK};
  int w = tft.width() / 5;
  for (int i = 0; i < 5; i++) tft.fillRect(i * w, 0, w, tft.height(), bars[i]);
  tft.drawRect(0, 0, tft.width(), tft.height(), ST77XX_WHITE);
  tft.setTextColor(ST77XX_YELLOW, ST77XX_BLACK);
  tft.setCursor(4, tft.height() - 12);
  tft.printf("R G B W K  %luMHz", spiHz / 1000000);
}

void begin() {
  tft.initR(tab);
  tft.setSPISpeed(spiHz);
  tft.setRotation(rotation);
  testPattern();
}

void drawFace(float t) {
  frame.fillScreen(ST77XX_BLACK);
  int cx = frame.width() / 2, cy = frame.height() / 2 - 6;
  int bob = (int)roundf(3 * sinf(t * 2.0f));
  bool blink = fmodf(t, 3.5f) < 0.15f;
  for (int side : {-1, 1}) {
    int ex = cx + side * 26, ey = cy + bob;
    if (blink) frame.fillRoundRect(ex - 14, ey - 2, 28, 5, 2, ST77XX_CYAN);
    else frame.fillRoundRect(ex - 12, ey - 18, 24, 36, 10, ST77XX_CYAN);
  }
  // Smile: a thick lower half-circle.
  for (int r = 10; r <= 13; r++)
    for (int a = 20; a <= 160; a += 4) {
      float rad = a * PI / 180;
      frame.drawPixel(cx + r * cosf(rad), cy + 30 + bob + r * sinf(rad), ST77XX_MAGENTA);
    }
  frame.setTextColor(0x7BEF);
  frame.setCursor(cx - 12, frame.height() - 14);
  frame.print("Dexi");
  tft.drawRGBBitmap(0, 0, frame.getBuffer(), frame.width(), frame.height());
}

void handleSerial() {
  if (!Serial.available()) return;
  String cmd = Serial.readStringUntil(' ');
  int v = Serial.parseInt();
  Serial.readStringUntil('\n');
  cmd.trim();
  if (cmd == "rot") rotation = v & 3;
  else if (cmd == "mhz") spiHz = v * 1000000UL;
  else if (cmd == "tab" && v >= 1 && v <= 6) tab = kTabs[v - 1];
  begin();
  Serial.printf("rot=%d mhz=%lu\n", rotation, spiHz / 1000000);
  delay(5000);
}

void setup() {
  Serial.begin(115200);
  pinMode(kBacklight, OUTPUT);
  digitalWrite(kBacklight, HIGH);
  begin();
  Serial.println("TFT up: bars should read R G B W K left to right");
  delay(8000);
}

void loop() {
  handleSerial();
  uint32_t start = millis();
  drawFace(start / 1000.0f);
  static uint32_t lastLog = 0;
  if (start - lastLog > 5000) {
    lastLog = start;
    Serial.printf("frame %lu ms\n", millis() - start);
  }
}
