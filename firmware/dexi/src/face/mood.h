// Dexi's moods and how each one looks.
#pragma once

#include <Arduino.h>

enum class Mood : uint8_t {
  kNeutral, kHappy, kExcited, kLove, kSleepy, kSad,
  kAngry, kSurprised, kHot, kCold, kCurious, kCount
};

// Geometry the face renderer interpolates between, so mood changes morph.
struct Look {
  float eyeW, eyeH, eyeRadius;
  float lidCut;      // 0..1 of the eye hidden by a flat upper lid
  float lidTilt;     // px: >0 inner corners down (angry), <0 outer down (sad)
  float mouthCurve;  // -1 frown .. +1 smile
  float mouthOpen;   // 0 closed .. 1 wide open
  float mouthW;      // half-width in px
  uint16_t color;    // RGB565
};

namespace mood {

const char* name(Mood m);
bool fromName(const String& s, Mood* out);
const Look& look(Mood m);

}  // namespace mood
