#include "touch.h"

Touch::Touch(uint8_t signalPin, uint8_t powerPin, uint8_t groundPin)
    : signal_(signalPin), power_(powerPin), ground_(groundPin) {}

void Touch::begin() {
  pinMode(power_, OUTPUT);
  digitalWrite(power_, HIGH);
  pinMode(ground_, OUTPUT);
  digitalWrite(ground_, LOW);
  pinMode(signal_, INPUT_PULLDOWN);  // reads LOW if the pad isn't connected
}

bool Touch::update(uint32_t now) {
  bool pressed = digitalRead(signal_);
  if (pressed && !down_) {
    down_ = true;
    downSince_ = now;
    if (current_.presses == 0) gestureStart_ = now;
  } else if (!pressed && down_) {
    down_ = false;
    uint32_t dur = now - downSince_;
    if (dur < kDebounceMs) return false;
    uint8_t i = current_.presses;
    if (i < Gesture::kMaxPresses) {
      current_.pressMs[i] = min<uint32_t>(dur, 65535);
      current_.gapMs[i] = i ? min<uint32_t>(downSince_ - lastRelease_, 65535) : 0;
    }
    current_.presses++;
    lastRelease_ = now;
  }
  if (!down_ && current_.presses && now - lastRelease_ > kGestureGapMs) {
    current_.totalMs = lastRelease_ - gestureStart_;
    current_.id = last_.id + 1;
    last_ = current_;
    current_ = Gesture();
    return true;
  }
  return false;
}
