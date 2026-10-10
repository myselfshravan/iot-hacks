#include "wifi_link.h"

#include <WiFi.h>

void WifiLink::begin(const char* hostname, const Network* networks, size_t count) {
  networks_ = networks;
  count_ = count;
  WiFi.mode(WIFI_STA);
  WiFi.setHostname(hostname);
  WiFi.setSleep(false);  // keeps the web server and Alexa responsive
}

void WifiLink::update() {
  bool up = WiFi.status() == WL_CONNECTED;
  if (up != online_) {
    online_ = up;
    if (up) {
      Serial.printf("Wi-Fi OK  IP=%s  RSSI=%d dBm\n", WiFi.localIP().toString().c_str(),
                    WiFi.RSSI());
      backoffMs_ = kMinBackoffMs;
      if (!everOnline_ && firstConnect_) firstConnect_();
      everOnline_ = true;
    } else {
      Serial.println("Wi-Fi lost");
    }
  }
  if (up) return;

  // Give each network kAttemptMs; after the last one, wait backoffMs_.
  uint32_t now = millis();
  bool roundDone = next_ == 0;
  uint32_t wait = roundDone ? backoffMs_ : kAttemptMs;
  if (lastTryMs_ && now - lastTryMs_ < wait) return;
  if (roundDone && lastTryMs_) backoffMs_ = min(backoffMs_ * 2, kMaxBackoffMs);
  lastTryMs_ = now;
  if (everOnline_) reconnects_++;

  const Network& n = networks_[next_];
  next_ = (next_ + 1) % count_;
  Serial.printf("Wi-Fi trying %s\n", n.ssid);
  WiFi.disconnect();
  WiFi.begin(n.ssid, n.password);
}
