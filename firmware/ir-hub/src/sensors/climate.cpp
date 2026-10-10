#include "climate.h"

Climate::Climate(uint8_t dataPin, uint8_t powerPin)
    : powerPin_(powerPin), dht_(dataPin, kType) {}

void Climate::begin() {
  pinMode(powerPin_, OUTPUT);
  digitalWrite(powerPin_, HIGH);
  dht_.begin();
  lastReadMs_ = millis();  // DHT22 needs ~2 s after power-up; first read at +5 s
}

const char* Climate::name(Confirm c) {
  switch (c) {
    case Confirm::kPending: return "pending";
    case Confirm::kConfirmed: return "confirmed";
    case Confirm::kUnconfirmed: return "unconfirmed";
    default: return "none";
  }
}

void Climate::expect(int direction) {
  if (direction == 0 || !ok()) {
    confirm_ = Confirm::kNone;
    return;
  }
  direction_ = direction;
  baselineC_ = tempC_;
  expectSinceMs_ = millis();
  confirm_ = Confirm::kPending;
}

void Climate::update() {
  uint32_t now = millis();
  if (lastReadMs_ && now - lastReadMs_ < kReadEveryMs) return;
  lastReadMs_ = now;

  float t = dht_.readTemperature(), h = dht_.readHumidity();
  if (isnan(t) || isnan(h)) {
    if (++badReads_ >= 3) tempC_ = humidity_ = NAN;  // tolerate the odd bad read
    return;
  }
  badReads_ = 0;
  tempC_ = t;
  humidity_ = h;

  if (confirm_ != Confirm::kPending) return;
  if ((tempC_ - baselineC_) * direction_ >= kReactionC) confirm_ = Confirm::kConfirmed;
  else if (now - expectSinceMs_ > kWindowMs) confirm_ = Confirm::kUnconfirmed;
}
