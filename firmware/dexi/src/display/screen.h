// The TFT plus an off-screen canvas. Everything is drawn into canvas() and
// pushed whole with present(), so animation never flickers.
#pragma once

#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>

class Screen {
 public:
  static constexpr int kWidth = 128, kHeight = 160;

  Screen(uint8_t cs, uint8_t dc, uint8_t rst, uint8_t backlight);

  void begin(uint8_t rotation, uint32_t spiHz);
  GFXcanvas16& canvas() { return canvas_; }
  void present();                         // pushes canvas()
  void present(const uint16_t* pixels);   // pushes a full 128x160 RGB565 image
  void setBacklight(bool on);

 private:
  Adafruit_ST7735 tft_;
  GFXcanvas16 canvas_;
  uint8_t backlight_;
};
