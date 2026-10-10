#include "screen.h"

#include <SPI.h>

Screen::Screen(uint8_t cs, uint8_t dc, uint8_t rst, uint8_t backlight)
    : tft_(&SPI, cs, dc, rst), canvas_(kWidth, kHeight), backlight_(backlight) {}

void Screen::begin(uint8_t rotation, uint32_t spiHz) {
  pinMode(backlight_, OUTPUT);
  setBacklight(true);
  // BLACKTAB is the variant this Robocraze ST7735R module needs
  // (found with experiments/12_tft_variants).
  tft_.initR(INITR_BLACKTAB);
  tft_.setSPISpeed(spiHz);
  tft_.setRotation(rotation);
  tft_.fillScreen(ST77XX_BLACK);
}

void Screen::present() { present(canvas_.getBuffer()); }

void Screen::present(const uint16_t* pixels) {
  tft_.drawRGBBitmap(0, 0, pixels, kWidth, kHeight);
}

void Screen::setBacklight(bool on) { digitalWrite(backlight_, on); }
