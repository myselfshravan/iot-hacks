// TTP223 touch pad as a gesture recorder. Instead of a fixed tap/double-tap
// vocabulary it records the whole gesture: every press duration and the gaps
// between presses. A gesture ends after kGestureGapMs without contact.
// The firmware reacts to its shape instantly; the brain gets the raw numbers
// and lets the LLM interpret them (a boop, nervous drumming, a long cuddle...).
#pragma once

#include <Arduino.h>

struct Gesture {
  static constexpr int kMaxPresses = 12;
  uint32_t id = 0;               // increments per finished gesture
  uint8_t presses = 0;           // total presses (may exceed kMaxPresses)
  uint16_t pressMs[kMaxPresses]; // duration of each recorded press
  uint16_t gapMs[kMaxPresses];   // gap before press i (gapMs[0] = 0)
  uint32_t totalMs = 0;          // first contact to last release
};

class Touch {
 public:
  Touch(uint8_t signalPin, uint8_t powerPin, uint8_t groundPin);
  void begin();

  // Returns true once when a gesture has just finished; read it with last().
  bool update(uint32_t now);

  bool held() const { return down_; }
  uint32_t heldMs(uint32_t now) const { return down_ ? now - downSince_ : 0; }
  bool active() const { return current_.presses > 0 || down_; }  // mid-gesture
  uint8_t pressesSoFar() const { return current_.presses; }
  const Gesture& last() const { return last_; }

 private:
  static constexpr uint32_t kGestureGapMs = 600, kDebounceMs = 25;

  uint8_t signal_, power_, ground_;
  bool down_ = false;
  uint32_t downSince_ = 0, lastRelease_ = 0, gestureStart_ = 0;
  Gesture current_, last_;
};
