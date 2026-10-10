// TTP223 capacitive touch pad: tap, double tap and long press.
// A single tap is reported only after the double-tap window has passed.
#pragma once

#include <Arduino.h>

class Touch {
 public:
  enum class Event { kNone, kTap, kDoubleTap, kLongPress };

  Touch(uint8_t signalPin, uint8_t powerPin, uint8_t groundPin, uint32_t longPressMs);
  void begin();
  Event update(uint32_t now);  // each event is returned once
  bool held() const { return down_; }

  uint32_t taps() const { return taps_; }
  uint32_t doubleTaps() const { return doubleTaps_; }
  uint32_t longPresses() const { return longPresses_; }

 private:
  static constexpr uint32_t kDoubleTapMs = 350;

  uint8_t signal_, power_, ground_;
  uint32_t longPressMs_;
  bool down_ = false, longFired_ = false, tapPending_ = false;
  uint32_t downSince_ = 0, lastRelease_ = 0;
  uint32_t taps_ = 0, doubleTaps_ = 0, longPresses_ = 0;
};
