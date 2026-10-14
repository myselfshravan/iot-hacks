#include "strip.h"

#include <Preferences.h>

#include "../config.h"

namespace {

constexpr const char* kModeNames[] = {"off", "mood", "rainbow", "solid", "chase", "sparkle", "candle"};
constexpr uint32_t kSyncTimeoutMs = 2000;
constexpr uint32_t kFrameMs = 20;  // 50 fps for mood/rainbow

// RGB565 body tint -> 8-bit channels.
void unpack565(uint16_t c, float out[3]) {
  out[0] = ((c >> 11) & 0x1F) * 255 / 31;
  out[1] = ((c >> 5) & 0x3F) * 255 / 63;
  out[2] = (c & 0x1F) * 255 / 31;
}

}  // namespace

Strip::Strip(uint8_t pin, uint16_t count, uint16_t skip, uint16_t maxMilliamps)
    : px_(count, pin, NEO_GRB + NEO_KHZ800), skip_(skip), maxMa_(maxMilliamps) {}

void Strip::begin() {
  Preferences p;
  p.begin("strip", true);
  uint8_t m = p.getUChar("mode", (uint8_t)Mode::kMood);
  mode_ = m < (uint8_t)Mode::kCount ? (Mode)m : Mode::kMood;
  brightness_ = p.getUChar("bri", brightness_);
  color_ = p.getULong("color", color_);
  sync_ = p.getBool("sync", sync_);
  p.end();
  flicker_ = (uint8_t*)calloc(px_.numPixels(), 1);
  px_.begin();
  px_.clear();
  px_.show();
}

void Strip::startNetwork() { listening_ = udp_.begin(config::kLedUdpPort); }

void Strip::setMode(Mode m) {
  mode_ = m;
  save();
}

void Strip::setBrightness(uint8_t b) {
  brightness_ = b;
  save();
}

void Strip::setColor(uint32_t rgb) {
  color_ = rgb & 0xFFFFFF;
  save();
}

void Strip::setSync(bool on) {
  sync_ = on;
  save();
}

void Strip::save() {
  Preferences p;
  p.begin("strip", false);
  p.putUChar("mode", (uint8_t)mode_);
  p.putUChar("bri", brightness_);
  p.putULong("color", color_);
  p.putBool("sync", sync_);
  p.end();
}

// Reads every pending HyperHDR packet; the last one wins. True if a frame arrived.
bool Strip::receive(uint32_t now) {
  if (!listening_) return false;
  bool got = false;
  uint32_t drained = 0;
  while (int len = udp_.parsePacket()) {
    drained++;
    uint16_t n = min<int>(len / 3, px_.numPixels() - skip_);
    uint32_t sum[3] = {0, 0, 0};
    for (uint16_t i = 0; i < n; i++) {
      uint8_t rgb[3];
      udp_.read(rgb, 3);
      px_.setPixelColor(skip_ + i, rgb[0], rgb[1], rgb[2]);
      for (int c = 0; c < 3; c++) sum[c] += rgb[c];
    }
    if (n) avg_ = (sum[0] / n) << 16 | (sum[1] / n) << 8 | (sum[2] / n);
    packets_++;
    udp_.flush();
    got = true;
  }
  if (got) lastPacket_ = now;
  queuedMax_ = max(queuedMax_, drained);
  return got;
}

void Strip::update(uint32_t now) {
  Mood mood = mood_;
  if (lastUpdate_) gapMax_ = max(gapMax_, now - lastUpdate_);
  lastUpdate_ = now;
  if (now - gapSince_ > 2000) {
    gapReport_ = gapMax_;
    queuedReport_ = queuedMax_;
    gapMax_ = queuedMax_ = 0;
    gapSince_ = now;
  }
  if (mode_ == Mode::kOff) {
    syncing_ = false;
    drain();
    if (now - lastFrame_ > 1000) {  // repaint now and then in case of a glitch
      lastFrame_ = now;
      px_.clear();
      px_.show();
    }
    return;
  }

  if (!sync_) {
    syncing_ = false;
    drain();
  } else if (receive(now)) {
    syncing_ = true;
    show(255);
    return;
  }
  syncing_ = syncing_ && now - lastPacket_ < kSyncTimeoutMs;
  if (syncing_ || now - lastFrame_ < kFrameMs) return;
  lastFrame_ = now;
  render(now, mood);
  for (uint16_t i = 0; i < skip_; i++) px_.setPixelColor(i, 0);
  show(brightness_);
}

void Strip::drain() {
  if (listening_) while (udp_.parsePacket()) udp_.flush();
}

void Strip::render(uint32_t now, Mood mood) {
  uint16_t n = px_.numPixels();
  uint8_t r = color_ >> 16, g = color_ >> 8, b = color_;
  switch (mode_) {
    case Mode::kSolid:
      px_.fill(px_.Color(r, g, b));
      return;
    case Mode::kChase: {
      // A comet with a 16-LED fading tail, one lap of the monitor every ~3 s.
      uint16_t len = n - skip_;
      uint16_t head = (now / 30) % len;
      px_.clear();
      for (uint16_t t = 0; t < 16; t++) {
        float k = 1.0f - t / 16.0f;
        k *= k;
        px_.setPixelColor(skip_ + (head + len - t) % len, r * k, g * k, b * k);
      }
      return;
    }
    case Mode::kSparkle: {
      for (uint16_t i = 0; i < n; i++) {  // fade everything a little
        uint32_t c = px_.getPixelColor(i);
        px_.setPixelColor(i, ((c >> 16) & 0xFF) * 7 / 8, ((c >> 8) & 0xFF) * 7 / 8, (c & 0xFF) * 7 / 8);
      }
      if (random(3) == 0) px_.setPixelColor(random(skip_, n), r, g, b);
      return;
    }
    case Mode::kCandle: {
      // Each LED drifts toward a random level; warm orange like a flame.
      for (uint16_t i = 0; i < n; i++) {
        uint8_t target = random(90, 256);
        flicker_[i] = (flicker_[i] * 6 + target * 2) / 8;
        uint8_t v = flicker_[i];
        px_.setPixelColor(i, v, v * 45 / 100, v * 8 / 100);
      }
      return;
    }
    default:
      break;
  }
  if (mode_ == Mode::kRainbow) {
    for (uint16_t i = 0; i < n; i++)
      px_.setPixelColor(i, px_.gamma32(px_.ColorHSV(now * 12 + i * 65536UL / n)));
  } else {
    // Ease toward the mood colour, breathe slowly; sleep is a dim slow breath.
    float target[3];
    unpack565(mood::style(mood).tint, target);
    for (int c = 0; c < 3; c++) glow_[c] += (target[c] - glow_[c]) * 0.05f;
    bool asleep = mood == Mood::kSleepy;
    float period = asleep ? 6000.0f : 4000.0f;
    float breath = 0.75f + 0.25f * sinf(now * TWO_PI / period);
    if (asleep) breath *= 0.3f;
    uint32_t c = px_.Color(px_.gamma8(glow_[0] * breath), px_.gamma8(glow_[1] * breath),
                           px_.gamma8(glow_[2] * breath));
    px_.fill(c);
  }
}

// Applies brightness, then scales down further if the frame would draw more
// than maxMa_ (about 20 mA per channel at full, plus ~1 mA idle per LED).
void Strip::show(uint8_t brightness) {
  uint16_t n = px_.numPixels();
  uint32_t sum = 0;
  for (uint16_t i = 0; i < n; i++) {
    uint32_t c = px_.getPixelColor(i);
    sum += ((c >> 16) & 0xFF) + ((c >> 8) & 0xFF) + (c & 0xFF);
  }
  uint32_t ma = sum * brightness / 255 * 20 / 255 + n;
  uint8_t bri = brightness;
  if (ma > maxMa_) bri = (uint32_t)brightness * (maxMa_ - n) / (ma - n);
  // setBrightness is lossy on stored pixels, so scale a copy instead.
  static uint8_t saved[3 * 512];
  uint8_t* raw = px_.getPixels();
  size_t bytes = min<size_t>(n * 3, sizeof(saved));
  memcpy(saved, raw, bytes);
  for (size_t i = 0; i < bytes; i++) raw[i] = raw[i] * bri / 255;
  px_.show();
  memcpy(raw, saved, bytes);
}

const char* Strip::modeName(Mode m) { return kModeNames[(uint8_t)m]; }

bool Strip::modeFromName(const String& s, Mode* out) {
  for (uint8_t i = 0; i < (uint8_t)Mode::kCount; i++)
    if (s == kModeNames[i]) {
      *out = (Mode)i;
      return true;
    }
  return false;
}
