#include "world.h"

#include <time.h>

namespace world {
namespace {

struct Rgb {
  float r, g, b;
};

uint16_t pack(const Rgb& c) {
  return ((uint8_t(c.r) & 0xF8) << 8) | ((uint8_t(c.g) & 0xFC) << 3) | (uint8_t(c.b) >> 3);
}

Rgb mix(const Rgb& a, const Rgb& b, float t) {
  return {a.r + (b.r - a.r) * t, a.g + (b.g - a.g) * t, a.b + (b.b - a.b) * t};
}

// Sky keyframes through the day: top colour, horizon colour.
struct Key {
  float hour;
  Rgb top, horizon;
};
const Key kKeys[] = {
    {0, {8, 10, 35}, {25, 25, 65}},       {5, {8, 10, 35}, {25, 25, 65}},
    {6.5, {70, 90, 170}, {255, 170, 120}}, {8, {80, 165, 255}, {175, 220, 255}},
    {17, {80, 165, 255}, {175, 220, 255}}, {18.5, {60, 50, 130}, {255, 130, 90}},
    {20, {8, 10, 35}, {25, 25, 65}},       {24, {8, 10, 35}, {25, 25, 65}},
};

const uint8_t kStars[][2] = {{8, 10},  {30, 24}, {52, 8},  {74, 30}, {96, 12}, {118, 26},
                             {16, 44}, {60, 50}, {104, 46}, {40, 66}, {86, 70}, {120, 60}};

void skyAt(float h, Rgb* top, Rgb* horizon) {
  for (size_t i = 1; i < sizeof(kKeys) / sizeof(kKeys[0]); i++) {
    if (h <= kKeys[i].hour) {
      const Key &a = kKeys[i - 1], &b = kKeys[i];
      float t = (h - a.hour) / (b.hour - a.hour);
      *top = mix(a.top, b.top, t);
      *horizon = mix(a.horizon, b.horizon, t);
      return;
    }
  }
}

// 0 at night, 1 in full day.
float daylight(float h) {
  if (h < 5 || h > 20) return 0;
  if (h < 8) return (h - 5) / 3;
  if (h > 17) return (20 - h) / 3;
  return 1;
}

}  // namespace

void startClock(const char* posixTz) { configTzTime(posixTz, "pool.ntp.org", "time.google.com"); }

float hour() {
  struct tm t;
  if (!getLocalTime(&t, 0)) return -1;
  return t.tm_hour + t.tm_min / 60.0f + t.tm_sec / 3600.0f;
}

void draw(GFXcanvas16& c, uint32_t now, bool showClock) {
  float h = hour();
  if (h < 0) h = 12;  // not synced yet: assume midday
  float light = daylight(h);

  Rgb top, horizon;
  skyAt(h, &top, &horizon);
  for (int y = 0; y < kGroundY; y++) c.drawFastHLine(0, y, c.width(), pack(mix(top, horizon, y / float(kGroundY))));

  // Stars fade in as daylight goes; each twinkles on its own beat.
  if (light < 0.6f)
    for (size_t i = 0; i < sizeof(kStars) / sizeof(kStars[0]); i++) {
      bool on = ((now / 400) + i * 7) % 11 != 0;
      if (on) c.drawPixel(kStars[i][0], kStars[i][1], pack(mix({255, 255, 220}, top, light / 0.6f)));
    }

  // Sun by day, moon by night, arcing across the sky.
  bool day = h >= 6 && h <= 18.5f;
  float t = day ? (h - 6) / 12.5f : fmodf(h + 5.5f, 24) / 11.0f;  // 0..1 across the sky
  int bx = 8 + t * 112, by = 70 - sinf(PI * t) * 52;
  if (day) {
    c.fillCircle(bx, by, 8, 0xFFE0);
    c.fillCircle(bx, by, 6, 0xFFF4);
  } else {
    c.fillCircle(bx, by, 7, 0xEF7D);
    c.fillCircle(bx + 3, by - 2, 6, pack(mix(top, horizon, by / float(kGroundY))));
  }

  // Grass and soil, darker at night.
  Rgb grass = mix({40, 70, 50}, {95, 190, 85}, light), soil = mix({30, 40, 35}, {70, 130, 60}, light);
  c.fillRect(0, kGroundY, c.width(), c.height() - kGroundY, pack(soil));
  c.drawFastHLine(0, kGroundY, c.width(), pack(grass));
  c.drawFastHLine(0, kGroundY + 1, c.width(), pack(grass));
  for (int x = 3; x < c.width(); x += 9) c.drawPixel(x, kGroundY - 1, pack(grass));  // blades

  if (showClock) {
    struct tm tmNow;
    if (getLocalTime(&tmNow, 0)) {
      char buf[6];
      strftime(buf, sizeof(buf), "%H:%M", &tmNow);
      c.setTextSize(1);
      c.setTextColor(0x0000);
      c.setCursor(c.width() - 31, 4);
      c.print(buf);
      c.setTextColor(0xFFFF);
      c.setCursor(c.width() - 32, 3);
      c.print(buf);
    }
  }
}

}  // namespace world
