// Dexi's moods and how each one looks and behaves.
#pragma once

#include <Arduino.h>

enum class Mood : uint8_t {
  kNeutral, kHappy, kExcited, kLove, kSleepy, kSad,
  kAngry, kSurprised, kHot, kCold, kCurious, kCount
};

enum class Eyes : uint8_t { kOpen, kBig, kClosed, kHappy, kHearts, kAngry, kSleepy };
enum class Mouth : uint8_t { kNone, kNeutral, kSmile, kOpen, kOh, kFrown, kTongue };

// What the pet does on its own while in a mood.
enum class Behaviour : uint8_t {
  kWander,  // strolls, idles, glances around
  kBouncy,  // wanders and hops a lot
  kSit,     // stays put, content
  kSleep,   // curls into a loaf
  kShiver,  // stays put, trembling
  kStomp,   // stays put, shaking with anger
};

struct MoodStyle {
  Eyes eyes;
  Mouth mouth;
  bool cheeks;
  Behaviour behaviour;
  uint16_t tint;   // body colour (RGB565)
  char mark;       // symbol floating above the head ('\0' = none)
};

namespace mood {

const char* name(Mood m);
bool fromName(const String& s, Mood* out);
const MoodStyle& style(Mood m);

}  // namespace mood
