#include "face.h"

namespace {

constexpr uint16_t kBg = 0x0000;
constexpr uint16_t kBlush = 0xFB56;
constexpr uint16_t kSweat = 0x5DFF;
constexpr uint32_t kBlinkMs = 140;
constexpr int kEyeGap = 24;    // eye centre distance from face centre
constexpr int kMouthDy = 34;   // mouth below eye line
constexpr float kMorph = 0.18; // per-frame approach to the target look

float approach(float cur, float target, float k) { return cur + (target - cur) * k; }

uint16_t blend(uint16_t a, uint16_t b, float t) {
  auto ch = [&](int shift, int mask) {
    int ca = (a >> shift) & mask, cb = (b >> shift) & mask;
    return ((int)(ca + (cb - ca) * t) & mask) << shift;
  };
  return ch(11, 0x1F) | ch(5, 0x3F) | ch(0, 0x1F);
}

void heart(GFXcanvas16& c, int x, int y, int r, uint16_t color) {
  c.fillCircle(x - r, y, r, color);
  c.fillCircle(x + r, y, r, color);
  c.fillTriangle(x - 2 * r, y + 1, x + 2 * r, y + 1, x, y + 3 * r, color);
}

}  // namespace

void Face::setMood(Mood m, uint32_t holdMs) {
  mood_ = m;
  if (holdMs) moodUntil_ = millis() + holdMs;
  else {
    base_ = m;
    moodUntil_ = 0;
  }
}

void Face::say(const String& text, uint32_t ms) {
  sayText_ = text;
  sayUntil_ = millis() + ms;
}

void Face::lookAt(float x, float y, uint32_t ms) {
  targetX_ = constrain(x, -1.0f, 1.0f);
  targetY_ = constrain(y, -1.0f, 1.0f);
  holdLookUntil_ = millis() + ms;
}

void Face::update(uint32_t now) {
  if (moodUntil_ && (int32_t)(now - moodUntil_) >= 0) {
    mood_ = base_;
    moodUntil_ = 0;
  }
  if (sayUntil_ && (int32_t)(now - sayUntil_) >= 0) {
    sayText_ = "";
    sayUntil_ = 0;
  }

  // Morph geometry toward the mood's look.
  const Look& t = mood::look(mood_);
  cur_.eyeW = approach(cur_.eyeW, t.eyeW, kMorph);
  cur_.eyeH = approach(cur_.eyeH, t.eyeH, kMorph);
  cur_.eyeRadius = approach(cur_.eyeRadius, t.eyeRadius, kMorph);
  cur_.lidCut = approach(cur_.lidCut, t.lidCut, kMorph);
  cur_.lidTilt = approach(cur_.lidTilt, t.lidTilt, kMorph);
  cur_.mouthCurve = approach(cur_.mouthCurve, t.mouthCurve, kMorph);
  cur_.mouthOpen = approach(cur_.mouthOpen, t.mouthOpen, kMorph);
  cur_.mouthW = approach(cur_.mouthW, t.mouthW, kMorph);
  cur_.color = blend(cur_.color, t.color, 0.25);

  // Idle glances: look somewhere new every few seconds, often back to centre.
  if ((int32_t)(now - holdLookUntil_) >= 0 && (int32_t)(now - nextGlance_) >= 0) {
    bool centre = random(100) < 40 || mood_ == Mood::kSleepy;
    targetX_ = centre ? 0 : random(-100, 101) / 100.0f;
    targetY_ = centre ? 0 : random(-60, 61) / 100.0f;
    if (mood_ == Mood::kCurious) targetY_ = -0.6;
    nextGlance_ = now + random(1500, 5000);
  }
  lookX_ = approach(lookX_, targetX_, 0.2);
  lookY_ = approach(lookY_, targetY_, 0.2);

  // Blinks at random intervals; sometimes a double blink.
  if ((int32_t)(now - nextBlink_) >= 0) {
    blinkStart_ = now;
    nextBlink_ = now + (random(100) < 20 ? 300 : random(2000, 6000));
  }
  lastUpdate_ = now;
}

void Face::draw(GFXcanvas16& c, uint32_t now) {
  c.fillScreen(kBg);
  float secs = now / 1000.0f;
  bool bubble = sayText_.length() > 0;

  float breath = sinf(secs * (mood_ == Mood::kSleepy ? 1.0f : 2.0f)) * 2.0f;
  int cx = c.width() / 2 + (int)(lookX_ * 6);
  int cy = (bubble ? 48 : 64) + (int)(breath + lookY_ * 5);
  if (mood_ == Mood::kCold) cx += random(-1, 2);  // shiver

  float blink = 1.0f;
  uint32_t sinceBlink = now - blinkStart_;
  if (sinceBlink < kBlinkMs) blink = 1.0f - sinf(PI * sinceBlink / kBlinkMs) * 0.9f;
  if (mood_ == Mood::kSleepy) blink = min(blink, 0.75f + 0.25f * sinf(secs * 0.7f));

  drawEye(c, cx - kEyeGap, cy, -1, blink);
  drawEye(c, cx + kEyeGap, cy, +1, blink);
  drawMouth(c, cx, cy + kMouthDy);
  drawExtras(c, cx, cy, now);
  if (bubble) drawBubble(c);
}

void Face::drawEye(GFXcanvas16& c, int ex, int ey, int side, float blink) {
  float w = cur_.eyeW, h = max(3.0f, cur_.eyeH * blink);
  if (mood_ == Mood::kCurious && side > 0) h *= 1.15f;  // one eyebrow-raise
  int x0 = ex - w / 2, y0 = ey - h / 2;
  int r = min<int>(cur_.eyeRadius, min(w, h) / 2);
  c.fillRoundRect(x0, y0, w, h, r, cur_.color);

  // Happy/love eyes: a dark bite from below turns them into smiling arcs.
  if (mood_ == Mood::kHappy || mood_ == Mood::kLove || mood_ == Mood::kExcited)
    c.fillCircle(ex, y0 + h + w * 0.25f, w * 0.62f, kBg);

  // Flat upper lid.
  int cut = cur_.lidCut * h;
  if (cut > 0) c.fillRect(x0 - 1, y0 - 1, w + 2, cut + 1, kBg);

  // Tilted lid: a background triangle over the top edge.
  float tilt = cur_.lidTilt;
  if (fabsf(tilt) > 0.5f) {
    int inner = side < 0 ? x0 + w : x0, outer = side < 0 ? x0 : x0 + w;
    int low = tilt > 0 ? inner : outer;  // which corner the lid comes down on
    int high = tilt > 0 ? outer : inner;
    int top = y0 + cut - 1;
    c.fillTriangle(high, top, low, top, low, top + fabsf(tilt) * 1.6f, kBg);
  }
}

void Face::drawMouth(GFXcanvas16& c, int mx, int my) {
  float w = cur_.mouthW, curve = cur_.mouthCurve, open = cur_.mouthOpen;
  for (int dx = -w; dx <= w; dx++) {
    float k = 1.0f - (dx * dx) / (w * w);   // 1 at centre, 0 at corners
    int y = my + curve * 6 * k;             // smile: centre dips
    int depth = open * 12 * k;
    if (depth > 1) c.drawFastVLine(mx + dx, y - 1, depth + 2, cur_.color);
    else c.fillRect(mx + dx, y - 1, 1, 3, cur_.color);
  }
}

void Face::drawExtras(GFXcanvas16& c, int cx, int cy, uint32_t now) {
  float secs = now / 1000.0f;
  switch (mood_) {
    case Mood::kLove:
    case Mood::kHappy:
      c.fillCircle(cx - 34, cy + 18, 4, kBlush);
      c.fillCircle(cx + 34, cy + 18, 4, kBlush);
      if (mood_ == Mood::kLove)
        for (int i = 0; i < 3; i++) {
          float p = fmodf(secs * 0.5f + i / 3.0f, 1.0f);  // 0..1 rise
          heart(c, 20 + i * 44, 40 - p * 40, 3, blend(kBlush, kBg, p));
        }
      break;
    case Mood::kSleepy:
      for (int i = 0; i < 3; i++) {
        float p = fmodf(secs * 0.4f + i / 3.0f, 1.0f);
        c.setTextColor(blend(cur_.color, kBg, p));
        c.setTextSize(1 + (i == 2));
        c.setCursor(cx + 30 + p * 14, cy - 26 - p * 24);
        c.print('z');
      }
      c.setTextSize(1);
      break;
    case Mood::kHot: {
      float p = fmodf(secs * 0.6f, 1.0f);
      int sx = cx + 40, sy = cy - 18 + p * 16;
      c.fillCircle(sx, sy + 3, 3, kSweat);
      c.fillTriangle(sx - 3, sy + 2, sx + 3, sy + 2, sx, sy - 4, kSweat);
      break;
    }
    case Mood::kSurprised:
    case Mood::kExcited:
      if ((now / 300) % 2) {
        c.drawFastHLine(cx - 52, cy - 22, 6, cur_.color);
        c.drawFastHLine(cx + 46, cy - 22, 6, cur_.color);
      }
      break;
    default:
      break;
  }
}

void Face::drawBubble(GFXcanvas16& c) {
  constexpr int kCharsPerLine = 19, kMaxLines = 4, kLineH = 10;
  String lines[kMaxLines];
  int n = 0;
  String rest = sayText_;
  while (rest.length() && n < kMaxLines) {
    int cut = rest.length() <= kCharsPerLine ? rest.length() : rest.lastIndexOf(' ', kCharsPerLine);
    if (cut <= 0) cut = min<int>(kCharsPerLine, rest.length());
    lines[n++] = rest.substring(0, cut);
    rest = rest.substring(cut);
    rest.trim();
  }
  int h = n * kLineH + 8, y = c.height() - h - 2;
  c.fillRoundRect(2, y, c.width() - 4, h, 6, 0xFFFF);
  c.fillTriangle(56, y, 70, y, 63, y - 6, 0xFFFF);
  c.setTextSize(1);
  c.setTextColor(0x0000);
  for (int i = 0; i < n; i++) {
    c.setCursor(7, y + 5 + i * kLineH);
    c.print(lines[i]);
  }
}
