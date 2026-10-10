#include "mood.h"

namespace mood {
namespace {

constexpr uint16_t rgb(uint8_t r, uint8_t g, uint8_t b) {
  return ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
}

const char* const kNames[] = {"neutral", "happy", "excited", "love", "sleepy", "sad",
                              "angry", "surprised", "hot", "cold", "curious"};

//                    eyeW eyeH  rad  lidCut tilt  curve open mouthW colour
const Look kLooks[] = {
    /* neutral   */ {22, 30, 8, 0.00, 0, 0.3, 0.0, 12, rgb(80, 220, 255)},
    /* happy     */ {24, 24, 10, 0.00, 0, 1.0, 0.2, 16, rgb(80, 220, 255)},
    /* excited   */ {26, 34, 11, 0.00, 0, 1.0, 0.8, 16, rgb(255, 220, 60)},
    /* love      */ {24, 24, 10, 0.00, 0, 0.9, 0.1, 12, rgb(255, 110, 180)},
    /* sleepy    */ {24, 28, 8, 0.65, 0, 0.1, 0.0, 8, rgb(120, 140, 220)},
    /* sad       */ {20, 26, 8, 0.15, -7, -0.8, 0.0, 12, rgb(90, 140, 255)},
    /* angry     */ {22, 24, 5, 0.20, 9, -0.6, 0.0, 14, rgb(255, 70, 60)},
    /* surprised */ {22, 38, 11, 0.00, 0, 0.0, 1.0, 7, rgb(240, 240, 255)},
    /* hot       */ {22, 26, 8, 0.25, -4, -0.2, 0.5, 12, rgb(255, 140, 50)},
    /* cold      */ {20, 28, 8, 0.10, 0, -0.4, 0.0, 10, rgb(160, 220, 255)},
    /* curious   */ {22, 32, 9, 0.00, 0, 0.2, 0.0, 8, rgb(120, 255, 170)},
};
static_assert(sizeof(kLooks) / sizeof(kLooks[0]) == (int)Mood::kCount, "one look per mood");

}  // namespace

const char* name(Mood m) { return kNames[(int)m]; }

bool fromName(const String& s, Mood* out) {
  for (int i = 0; i < (int)Mood::kCount; i++)
    if (s.equalsIgnoreCase(kNames[i])) {
      *out = (Mood)i;
      return true;
    }
  return false;
}

const Look& look(Mood m) { return kLooks[(int)m]; }

}  // namespace mood
