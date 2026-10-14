// WS2812B strip behind the monitor, driven by Dexi's board.
//
// Modes (kept across reboots):
//   mood     glows in Dexi's body colour and breathes; dims while it sleeps
//   rainbow  test pattern
//   off
// In any mode except off, colours streamed by HyperHDR on the Mac take over
// (screen sync) and the strip falls back to its mode 2 s after the stream stops.
// HyperHDR device: "udpraw", host dexi.local, port config::kLedUdpPort, one RGB
// triplet per LED in strip order. Wiring: docs/wiring.md (Dexi LED strip).
#pragma once

#include <Adafruit_NeoPixel.h>
#include <WiFiUdp.h>

#include "../pet/mood.h"

class Strip {
 public:
  enum class Mode : uint8_t { kOff, kMood, kRainbow, kCount };

  Strip(uint8_t pin, uint16_t count, uint16_t maxMilliamps);
  void begin();                 // restores mode and brightness
  void startNetwork();          // after Wi-Fi is up: listen for HyperHDR
  void update(uint32_t now, Mood mood);

  void setMode(Mode m);
  void setBrightness(uint8_t b);
  Mode mode() const { return mode_; }
  uint8_t brightness() const { return brightness_; }
  bool syncing() const { return syncing_; }

  static const char* modeName(Mode m);
  static bool modeFromName(const String& s, Mode* out);

 private:
  void show();
  bool receive(uint32_t now);
  void save();

  Adafruit_NeoPixel px_;
  WiFiUDP udp_;
  uint16_t maxMa_;
  bool listening_ = false;
  bool syncing_ = false;
  uint32_t lastPacket_ = 0;
  uint32_t lastFrame_ = 0;
  Mode mode_ = Mode::kMood;
  uint8_t brightness_ = 160;
  float glow_[3] = {0, 0, 0};   // eased mood colour
};
