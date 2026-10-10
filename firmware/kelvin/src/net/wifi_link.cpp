#include "wifi_link.h"

#include <WiFi.h>

void WifiLink::begin(const char* hostname, const char* ssid, const char* password) {
  ssid_ = ssid;
  password_ = password;
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

  uint32_t now = millis();
  if (lastTryMs_ && now - lastTryMs_ < backoffMs_) return;
  lastTryMs_ = now;
  if (everOnline_) reconnects_++;
  Serial.printf("Wi-Fi connecting to %s (retry in %lus)\n", ssid_, backoffMs_ / 1000);
  WiFi.disconnect();
  WiFi.begin(ssid_, password_);
  backoffMs_ = min(backoffMs_ * 2, kMaxBackoffMs);
}
