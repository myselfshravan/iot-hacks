// Finds which ST7735 init variant this 1.8" module needs.
// Cycles Adafruit's init tabs every 8 s, each showing its name, R/G/B/W/K bars
// and a 1-px white border. The right variant has solid bars and all 4 edges.
// Wiring: same as 11_tft_bringup (docs/wiring.md, Dexi).

#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>
#include <SPI.h>

constexpr int kSck = 18, kMosi = 23, kCs = 5, kDc = 16, kRst = 17, kBacklight = 4;

Adafruit_ST7735 tft(kCs, kDc, kMosi, kSck, kRst);  // software SPI: slow but robust

struct Variant {
  const char* name;
  uint8_t tab;
};
const Variant kVariants[] = {
    {"BLACKTAB", INITR_BLACKTAB},
    {"GREENTAB", INITR_GREENTAB},
    {"REDTAB", INITR_REDTAB},
    {"18GREEN", INITR_18GREENTAB},
    {"18RED", INITR_18REDTAB},
    {"18BLACK", INITR_18BLACKTAB},
};

void show(const Variant& v, int index) {
  digitalWrite(kRst, LOW);
  delay(20);
  tft.initR(v.tab);
  tft.setRotation(0);
  const uint16_t bars[] = {ST77XX_RED, ST77XX_GREEN, ST77XX_BLUE, ST77XX_WHITE, ST77XX_BLACK};
  int w = tft.width() / 5;
  for (int i = 0; i < 5; i++) tft.fillRect(i * w, 0, w, tft.height(), bars[i]);
  tft.drawRect(0, 0, tft.width(), tft.height(), ST77XX_WHITE);
  tft.fillRect(10, 60, 108, 30, ST77XX_BLACK);
  tft.setTextColor(ST77XX_YELLOW);
  tft.setTextSize(1);
  tft.setCursor(16, 64);
  tft.printf("%d: %s", index + 1, v.name);
  tft.setCursor(16, 78);
  tft.print("R G B W K");
  Serial.printf("showing %d: %s\n", index + 1, v.name);
}

void setup() {
  Serial.begin(115200);
  pinMode(kBacklight, OUTPUT);
  digitalWrite(kBacklight, HIGH);
}

void loop() {
  static int i = 0;
  show(kVariants[i], i);
  i = (i + 1) % (sizeof(kVariants) / sizeof(kVariants[0]));
  delay(8000);
}
