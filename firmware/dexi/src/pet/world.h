// The little world Dexi lives in: a sky that follows the real time of day
// (via NTP), sun/moon, twinkling stars at night, grass, and a small clock.
#pragma once

#include <Adafruit_GFX.h>

namespace world {

constexpr int kGroundY = 136;  // feet rest on this line

// Starts NTP time sync (call once Wi-Fi is up).
void startClock(const char* posixTz);

// Draws sky, sun/moon, stars and ground. Clock hidden while `showClock` is false.
void draw(GFXcanvas16& c, uint32_t now, bool showClock);

// Local hour as a fraction (e.g. 13.5), or -1 before the clock is synced.
float hour();

}  // namespace world
