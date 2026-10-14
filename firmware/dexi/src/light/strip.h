// WS2812B strip behind the monitor, driven by Dexi's board.
//
// Modes (kept across reboots, with colour, brightness and the sync switch):
//   mood     glows in Dexi's body colour and breathes; dims while it sleeps
//   rainbow  flowing rainbow
//   solid    one colour (setColor)
//   chase    a comet of the colour running round the monitor
//   sparkle  twinkles of the colour on black
//   candle   warm flickering firelight
//   off      everything dark, stream ignored
// With sync on, colours streamed by HyperHDR on the Mac take over (screen sync,
// always at full brightness under the current cap) and the strip falls back to
// its mode 2 s after the stream stops. Sync off: the mode shows, stream ignored.
// The first `skip` LEDs sit outside the monitor and stay dark.
// HyperHDR device: "udpraw", host dexi.local, port config::kLedUdpPort, one RGB
// triplet per LED in strip order. Wiring: docs/wiring.md (Dexi LED strip).
#pragma once

#include <Adafruit_NeoPixel.h>
#include <WiFiUdp.h>

#include "../pet/mood.h"

class Strip {
 public:
  // Order is stored in flash: append only.
  enum class Mode : uint8_t { kOff, kMood, kRainbow, kSolid, kChase, kSparkle, kCandle, kCount };

  Strip(uint8_t pin, uint16_t count, uint16_t skip, uint16_t maxMilliamps);
  void begin();                 // restores mode and brightness
  void startNetwork();          // after Wi-Fi is up: listen for HyperHDR
  void setMood(Mood m) { mood_ = m; }
  void update(uint32_t now);    // call every loop()

  void setMode(Mode m);
  void setBrightness(uint8_t b);
  void setColor(uint32_t rgb);
  void setSync(bool on);
  Mode mode() const { return mode_; }
  uint8_t brightness() const { return brightness_; }
  uint32_t color() const { return color_; }
  bool syncEnabled() const { return sync_; }
  bool syncing() const { return syncing_; }
  uint32_t packets() const { return packets_; }
  uint32_t loopGapMs() const { return gapReport_; }  // worst gap between update() calls, last 2 s
  uint32_t queued() const { return queuedReport_; } // most packets drained at once, last 2 s
  uint32_t lastAverage() const { return avg_; }  // 0xRRGGBB of the last synced frame

  static const char* modeName(Mode m);
  static bool modeFromName(const String& s, Mode* out);

 private:
  void show(uint8_t brightness);
  bool receive(uint32_t now);
  void drain();
  void render(uint32_t now, Mood mood);
  void save();

  Adafruit_NeoPixel px_;
  WiFiUDP udp_;
  uint16_t skip_;
  uint16_t maxMa_;
  volatile bool listening_ = false;
  volatile bool syncing_ = false;
  volatile uint32_t packets_ = 0;
  volatile uint32_t avg_ = 0;
  uint32_t lastUpdate_ = 0, gapMax_ = 0, gapReport_ = 0, gapSince_ = 0;
  uint32_t queuedMax_ = 0, queuedReport_ = 0;
  volatile Mood mood_ = Mood::kNeutral;
  uint32_t lastPacket_ = 0;
  uint32_t lastFrame_ = 0;
  volatile Mode mode_ = Mode::kMood;
  volatile uint8_t brightness_ = 160;
  volatile uint32_t color_ = 0xFF6A00;  // amber
  volatile bool sync_ = true;
  uint8_t* flicker_ = nullptr;  // candle: per-LED level
  float glow_[3] = {0, 0, 0};   // eased mood colour
};
