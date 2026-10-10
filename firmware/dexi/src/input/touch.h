// TTP223 capacitive touch pad: debounced taps and long presses.
#pragma once

#include <Arduino.h>

class Touch {
 public:
  enum class Event { kNone, kTap, kLongPress };

  Touch(uint8_t signalPin, uint8_t powerPin, uint8_t groundPin, uint32_t longPressMs);
  void begin();
  Event update(uint32_t now);  // returns an event once, on release / threshold

  uint32_t taps() const { return taps_; }
  uint32_t longPresses() const { return longPresses_; }

 private:
  uint8_t signal_, power_, ground_;
  uint32_t longPressMs_;
  bool down_ = false, longFired_ = false;
  uint32_t downSince_ = 0, taps_ = 0, longPresses_ = 0;
};
