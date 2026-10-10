// 1.8" ST7735 TFT (128x160, SPI) bring-up for Dexi.
// Shows colour bars + corner markers (to check colour order and pixel
// offsets), then a blinking face drawn into an off-screen sprite (to check
// that animation is smooth and flicker-free).
//
// Wiring (TFT -> ESP32), all on the ESP32's left header (docs/wiring.md, Dexi):
//   VCC -> 3V3    GND -> GND    SCK/SCL -> D18    SDA/MOSI -> D23
//   CS -> D5      A0/DC -> D16  RESET/RST -> D17  LED/BL -> D4
//
// Serial: "offset <x> <y>", "bgr 0|1", "invert 0|1" to tune the panel live
// (variants of these modules differ); then hard-code the values.

#include <Arduino.h>
#include <LovyanGFX.hpp>

constexpr int kSck = 18, kMosi = 23, kCs = 5, kDc = 16, kRst = 17, kBacklight = 4;

class Tft : public lgfx::LGFX_Device {
 public:
  lgfx::Panel_ST7735S panel;
  lgfx::Bus_SPI bus;
  lgfx::Light_PWM light;

  Tft() {
    auto b = bus.config();
    b.spi_host = VSPI_HOST;
    b.spi_mode = 0;
    b.freq_write = 27000000;
    b.pin_sclk = kSck;
    b.pin_mosi = kMosi;
    b.pin_miso = -1;
    b.pin_dc = kDc;
    bus.config(b);
    panel.setBus(&bus);

    auto p = panel.config();
    p.pin_cs = kCs;
    p.pin_rst = kRst;
    p.panel_width = 128;
    p.panel_height = 160;
    p.memory_width = 132;
    p.memory_height = 162;
    p.offset_x = 2;
    p.offset_y = 1;
    p.rgb_order = false;
    p.invert = false;
    panel.config(p);

    auto l = light.config();
    l.pin_bl = kBacklight;
    l.pwm_channel = 7;
    light.config(l);
    panel.setLight(&light);
    setPanel(&panel);
  }
};

Tft tft;
LGFX_Sprite frame(&tft);

void testPattern() {
  const uint16_t bars[] = {TFT_RED, TFT_GREEN, TFT_BLUE, TFT_WHITE, TFT_BLACK};
  int w = tft.width() / 5;
  for (int i = 0; i < 5; i++) tft.fillRect(i * w, 0, w, tft.height(), bars[i]);
  // 1-px white frame: every edge must be visible, none doubled or cut.
  tft.drawRect(0, 0, tft.width(), tft.height(), TFT_WHITE);
  tft.setTextColor(TFT_YELLOW, TFT_BLACK);
  tft.setCursor(4, tft.height() - 12);
  tft.print("R G B W K");
}

void drawFace(float t) {
  frame.fillScreen(TFT_BLACK);
  int cx = frame.width() / 2, cy = frame.height() / 2 - 6;
  int bob = (int)(3 * sinf(t * 2.0f));
  bool blink = fmodf(t, 3.5f) < 0.15f;
  for (int side : {-1, 1}) {
    int ex = cx + side * 26, ey = cy + bob;
    if (blink) frame.fillRoundRect(ex - 14, ey - 2, 28, 5, 2, TFT_CYAN);
    else frame.fillRoundRect(ex - 12, ey - 18, 24, 36, 10, TFT_CYAN);
  }
  frame.fillArc(cx, cy + 30 + bob, 12, 9, 20, 160, TFT_PINK);
  frame.setTextColor(TFT_DARKGREY);
  frame.drawCenterString("Dexi", cx, frame.height() - 14);
  frame.pushSprite(0, 0);
}

void handleSerial() {
  if (!Serial.available()) return;
  String cmd = Serial.readStringUntil(' ');
  cmd.trim();
  auto cfg = tft.panel.config();
  if (cmd == "offset") {
    cfg.offset_x = Serial.parseInt();
    cfg.offset_y = Serial.parseInt();
  } else if (cmd == "bgr") {
    cfg.rgb_order = Serial.parseInt();
  } else if (cmd == "invert") {
    cfg.invert = Serial.parseInt();
  }
  Serial.readStringUntil('\n');
  tft.panel.config(cfg);
  tft.init();
  testPattern();
  Serial.printf("offset=%d,%d bgr=%d invert=%d\n", cfg.offset_x, cfg.offset_y,
                cfg.rgb_order, cfg.invert);
  delay(4000);
}

void setup() {
  Serial.begin(115200);
  tft.init();
  tft.setBrightness(200);
  tft.setRotation(0);
  testPattern();
  Serial.println("TFT up: check bars are Red Green Blue White Black and the white frame");
  delay(4000);
  frame.createSprite(tft.width(), tft.height());
}

void loop() {
  handleSerial();
  drawFace(millis() / 1000.0f);
}
