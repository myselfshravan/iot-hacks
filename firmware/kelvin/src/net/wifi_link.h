// Keeps the station connected without ever blocking loop(): reconnects with
// backoff (5 s doubling to 60 s) and fires a callback on the first connect.
#pragma once

#include <Arduino.h>

class WifiLink {
 public:
  using Callback = void (*)();

  void begin(const char* hostname, const char* ssid, const char* password);
  void onFirstConnect(Callback fn) { firstConnect_ = fn; }
  void update();  // call every loop

  bool online() const { return online_; }
  uint32_t reconnects() const { return reconnects_; }

 private:
  const char* ssid_ = nullptr;
  const char* password_ = nullptr;
  Callback firstConnect_ = nullptr;
  bool online_ = false, everOnline_ = false;
  uint32_t lastTryMs_ = 0, backoffMs_ = kMinBackoffMs, reconnects_ = 0;

  static constexpr uint32_t kMinBackoffMs = 5000, kMaxBackoffMs = 60000;
};
