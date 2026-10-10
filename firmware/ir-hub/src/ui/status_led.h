// Onboard blue LED (GPIO 2) as a health light:
//   solid on   = Wi-Fi connected, hub online
//   slow blink = Wi-Fi down / reconnecting
//   fast blink = IR being sent (for ~400 ms)
#pragma once

#include <Arduino.h>

class StatusLed {
 public:
  explicit StatusLed(uint8_t pin) : pin_(pin) {}

  void begin() { pinMode(pin_, OUTPUT); }
  void flashTx() { txUntil_ = millis() + 400; }

  void update(bool online) {
    uint32_t now = millis();
    bool on;
    if ((int32_t)(txUntil_ - now) > 0) on = (now / 50) % 2;
    else if (online) on = true;
    else on = (now / 500) % 2;
    digitalWrite(pin_, on);
  }

 private:
  uint8_t pin_;
  uint32_t txUntil_ = 0;
};
