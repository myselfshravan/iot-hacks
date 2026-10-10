// Keeps the station connected without ever blocking loop(). Knows several
// networks (home town, Bangalore flat...) and tries them in turn; after a
// full round with no luck it backs off (5 s doubling to 60 s). Fires a
// callback on the first connect.
#pragma once

#include <Arduino.h>

class WifiLink {
 public:
  using Callback = void (*)();
  struct Network {
    const char* ssid;
    const char* password;
  };

  void begin(const char* hostname, const Network* networks, size_t count);
  void onFirstConnect(Callback fn) { firstConnect_ = fn; }
  void update();  // call every loop

  bool online() const { return online_; }
  uint32_t reconnects() const { return reconnects_; }

 private:
  const Network* networks_ = nullptr;
  size_t count_ = 0, next_ = 0;
  Callback firstConnect_ = nullptr;
  bool online_ = false, everOnline_ = false;
  uint32_t lastTryMs_ = 0, backoffMs_ = kMinBackoffMs, reconnects_ = 0;

  static constexpr uint32_t kAttemptMs = 10000;  // per network
  static constexpr uint32_t kMinBackoffMs = 5000, kMaxBackoffMs = 60000;
};
