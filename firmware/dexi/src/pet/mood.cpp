#include "mood.h"

namespace mood {
namespace {

constexpr uint16_t rgb(uint8_t r, uint8_t g, uint8_t b) {
  return ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
}

constexpr uint16_t kGinger = rgb(255, 160, 70);

const char* const kNames[] = {"neutral", "happy", "excited", "love", "sleepy", "sad",
                              "angry", "surprised", "hot", "cold", "curious"};

//                      eyes           mouth           cheeks behaviour            tint                 mark
const MoodStyle kStyles[] = {
    /* neutral   */ {Eyes::kOpen,   Mouth::kNeutral, false, Behaviour::kWander, kGinger,             0},
    /* happy     */ {Eyes::kHappy,  Mouth::kSmile,   true,  Behaviour::kWander, kGinger,             0},
    /* excited   */ {Eyes::kBig,    Mouth::kOpen,    true,  Behaviour::kBouncy, kGinger,             '!'},
    /* love      */ {Eyes::kHearts, Mouth::kSmile,   true,  Behaviour::kSit,    kGinger,             0},
    /* sleepy    */ {Eyes::kSleepy, Mouth::kNone,    false, Behaviour::kSleep,  kGinger,             0},
    /* sad       */ {Eyes::kOpen,   Mouth::kFrown,   false, Behaviour::kWander, rgb(200, 150, 110),  0},
    /* angry     */ {Eyes::kAngry,  Mouth::kFrown,   false, Behaviour::kStomp,  rgb(255, 90, 60),    '#'},
    /* surprised */ {Eyes::kBig,    Mouth::kOh,      false, Behaviour::kSit,    kGinger,             '!'},
    /* hot       */ {Eyes::kOpen,   Mouth::kTongue,  true,  Behaviour::kSit,    rgb(255, 120, 70),   0},
    /* cold      */ {Eyes::kClosed, Mouth::kFrown,   false, Behaviour::kShiver, rgb(170, 200, 255),  0},
    /* curious   */ {Eyes::kBig,    Mouth::kNeutral, false, Behaviour::kWander, kGinger,             '?'},
};
static_assert(sizeof(kStyles) / sizeof(kStyles[0]) == (int)Mood::kCount, "one style per mood");

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

const MoodStyle& style(Mood m) { return kStyles[(int)m]; }

}  // namespace mood
