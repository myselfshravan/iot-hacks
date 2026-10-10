#include "touch.h"

Touch::Touch(uint8_t signalPin, uint8_t powerPin, uint8_t groundPin, uint32_t longPressMs)
    : signal_(signalPin), power_(powerPin), ground_(groundPin), longPressMs_(longPressMs) {}

void Touch::begin() {
  pinMode(power_, OUTPUT);
  digitalWrite(power_, HIGH);
  pinMode(ground_, OUTPUT);
  digitalWrite(ground_, LOW);
  pinMode(signal_, INPUT_PULLDOWN);  // reads LOW if the pad isn't connected
}

Touch::Event Touch::update(uint32_t now) {
  bool pressed = digitalRead(signal_);
  if (pressed && !down_) {
    down_ = true;
    longFired_ = false;
    downSince_ = now;
  } else if (pressed && !longFired_ && now - downSince_ >= longPressMs_) {
    longFired_ = true;
    tapPending_ = false;
    longPresses_++;
    return Event::kLongPress;
  } else if (!pressed && down_) {
    down_ = false;
    if (longFired_ || now - downSince_ < 30) return Event::kNone;  // long press or glitch
    if (tapPending_ && now - lastRelease_ <= kDoubleTapMs) {
      tapPending_ = false;
      doubleTaps_++;
      return Event::kDoubleTap;
    }
    tapPending_ = true;
    lastRelease_ = now;
  }
  if (tapPending_ && !down_ && now - lastRelease_ > kDoubleTapMs) {
    tapPending_ = false;
    taps_++;
    return Event::kTap;
  }
  return Event::kNone;
}
