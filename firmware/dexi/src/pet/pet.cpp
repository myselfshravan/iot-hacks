#include "pet.h"

#include "sprites.h"
#include "world.h"

namespace {

constexpr int kScale = 4;                          // screen px per art pixel
constexpr int kArtW = sprites::kCols * kScale;     // 64
constexpr int kArtH = sprites::kRows * kScale;     // 48
constexpr int kMinX = kArtW / 2 + 4, kMaxX = 128 - kArtW / 2 - 4;
constexpr uint32_t kHopMs = 450, kBlinkMs = 120;

constexpr uint16_t kInk = 0x18E3, kPink = 0xFC96, kWhite = 0xFFFF;
constexpr uint16_t kTear = 0x5DFF, kHeart = 0xF8B2, kSteam = 0xC618;

// Lighter / darker versions of the body tint for highlight and shade.
uint16_t shadeOf(uint16_t c, float k) {
  int r = ((c >> 11) & 0x1F) * k, g = ((c >> 5) & 0x3F) * k, b = (c & 0x1F) * k;
  return (min(r, 31) << 11) | (min(g, 63) << 5) | min(b, 31);
}

uint16_t cellColour(char code, uint16_t tint) {
  switch (code) {
    case 'o': return kInk;
    case 'b': return tint;
    case 'h': return shadeOf(tint, 1.18f);
    case 'd': return shadeOf(tint, 0.78f);
    case 'p': return kPink;
    case 'w': return kWhite;
    default: return 0;
  }
}

void heart(GFXcanvas16& c, int x, int y, uint16_t colour) {
  c.fillRect(x, y, 2, 2, colour);
  c.fillRect(x + 3, y, 2, 2, colour);
  c.fillRect(x, y + 1, 5, 2, colour);
  c.fillRect(x + 1, y + 3, 3, 1, colour);
  c.drawPixel(x + 2, y + 4, colour);
}

}  // namespace

void Pet::setMood(Mood m, uint32_t holdMs) {
  mood_ = m;
  if (holdMs) moodUntil_ = millis() + holdMs;
  else {
    base_ = m;
    moodUntil_ = 0;
  }
  nextDecision_ = 0;  // react to the new mood right away
}

void Pet::say(const String& text, uint32_t ms) {
  sayText_ = text;
  sayUntil_ = millis() + ms;
}

void Pet::lookAt(float x, float y, uint32_t ms) {
  lookX_ = constrain(x, -1.0f, 1.0f);
  lookY_ = constrain(y, -1.0f, 1.0f);
  holdLookUntil_ = millis() + ms;
}

void Pet::hop() {
  if (millis() - hopStart_ > kHopMs) hopStart_ = millis();
}

// Picks the next thing to do on its own, depending on the mood's behaviour.
void Pet::decide(uint32_t now) {
  Behaviour b = mood::style(mood_).behaviour;
  walking_ = false;
  if (b == Behaviour::kWander || b == Behaviour::kBouncy) {
    int roll = random(100);
    if (roll < 55) {
      targetX_ = random(kMinX, kMaxX + 1);
      walking_ = fabsf(targetX_ - x_) > 6;
    }
    if (b == Behaviour::kBouncy || (mood_ == Mood::kHappy && roll > 85)) hop();
  }
  nextDecision_ = now + (b == Behaviour::kBouncy ? random(600, 1500) : random(2000, 6000));
}

void Pet::update(uint32_t now) {
  float dt = lastUpdate_ ? (now - lastUpdate_) / 1000.0f : 0;
  lastUpdate_ = now;
  if (moodUntil_ && (int32_t)(now - moodUntil_) >= 0) {
    mood_ = base_;
    moodUntil_ = 0;
  }
  if (sayUntil_ && (int32_t)(now - sayUntil_) >= 0) {
    sayText_ = "";
    sayUntil_ = 0;
  }
  if ((int32_t)(now - nextDecision_) >= 0) decide(now);

  if (walking_) {
    float speed = mood_ == Mood::kSad ? 8 : 20;  // px per second
    float step = speed * dt;
    facing_ = targetX_ > x_ ? 1 : -1;
    if (fabsf(targetX_ - x_) <= step) {
      x_ = targetX_;
      walking_ = false;
    } else {
      x_ += facing_ * step;
    }
  }

  // Idle glances when nobody is steering the eyes.
  if ((int32_t)(now - holdLookUntil_) >= 0 && (int32_t)(now - nextGlance_) >= 0) {
    bool centre = random(100) < 45;
    lookX_ = centre ? 0 : random(-1, 2);
    lookY_ = centre ? 0 : random(-1, 1);
    nextGlance_ = now + random(1200, 4000);
  }
  if ((int32_t)(now - nextBlink_) >= 0) {
    blinkStart_ = now;
    nextBlink_ = now + (random(100) < 15 ? 260 : random(2500, 6000));
  }
}

void Pet::draw(GFXcanvas16& c, uint32_t now) {
  bool talking = sayText_.length() > 0;
  world::draw(c, now, !talking);

  const MoodStyle& st = mood::style(mood_);
  int x = x_;
  if (st.behaviour == Behaviour::kShiver) x += random(-1, 2);
  if (st.behaviour == Behaviour::kStomp) x += ((now / 90) % 2) ? 1 : -1;

  uint32_t sinceHop = now - hopStart_;
  int hopY = sinceHop < kHopMs ? sinf(PI * sinceHop / kHopMs) * 16 : 0;
  int breath = ((now / 900) % 2) && !walking_ ? 1 : 0;

  if (pack_ && pack_->loaded()) {
    bool thinking = (int32_t)(thinkUntil_ - now) > 0;
    int slot = thinking ? PetPack::kThinkingSlot : (int)mood_;
    // Packs animate in place (sliding an already-animated sprite looks glitchy).
    pack_->draw(c, slot, c.width() / 2, world::kGroundY + 2, now);
    if (talking) drawBubble(c, c.width() / 2);
    return;
  }
  drawBuiltin(c, now, x, hopY, breath, talking);
}

void Pet::drawBuiltin(GFXcanvas16& c, uint32_t now, int x, int hopY, int breath, bool talking) {
  const MoodStyle& st = mood::style(mood_);
  bool asleep = st.behaviour == Behaviour::kSleep;
  int sink = asleep ? 2 * kScale : 0;  // asleep: tucks its legs under

  // Soft shadow on the grass; shrinks while airborne.
  int shadowW = 26 - hopY / 2;
  c.fillRoundRect(x - shadowW, world::kGroundY - 1, shadowW * 2, 4, 2, 0x2945);

  // Squash and stretch: tall while rising, flat for a beat after landing,
  // and a little bounce in each step while walking.
  uint32_t sinceHop = now - hopStart_;
  float sy = 1.0f;
  if (sinceHop < kHopMs / 2) sy = 1.12f;
  else if (sinceHop >= kHopMs && sinceHop < kHopMs + 120) sy = 0.82f;
  if (walking_) breath = ((now / 180) % 2) ? -2 : 0;
  int h = kArtH * sy, w = kArtW * (2.0f - sy);
  int x0 = x - w / 2, y0 = world::kGroundY - h - hopY + breath + sink;
  drawCat(c, x0, y0, w / (float)kArtW, h / (float)kArtH, now);

  int headY = y0;
  drawEffects(c, x, headY, now);
  if (talking) drawBubble(c, x);
}

void Pet::drawCat(GFXcanvas16& c, int x0, int y0, float sx, float sy, uint32_t now) {
  const MoodStyle& st = mood::style(mood_);
  bool asleep = st.behaviour == Behaviour::kSleep;
  // Cells are scaled by sx/sy for squash and stretch; edges are rounded so
  // neighbouring cells always meet without gaps.
  auto cell = [&](int col, int row, char code) {
    if (code == '.') return;
    int xa = x0 + (int)roundf(col * kScale * sx), xb = x0 + (int)roundf((col + 1) * kScale * sx);
    int ya = y0 + (int)roundf(row * kScale * sy), yb = y0 + (int)roundf((row + 1) * kScale * sy);
    c.fillRect(xa, ya, xb - xa, yb - ya, cellColour(code, st.tint));
  };

  // Body, then legs (hidden while asleep: it sits flat on the grass).
  for (int r = 0; r < 10; r++)
    for (int col = 0; col < sprites::kCols; col++) cell(col, r, sprites::kBody[r][col]);
  if (!asleep) {
    int legs = walking_ ? 1 + (now / 180) % 2 : 0;
    for (int r = 0; r < 2; r++)
      for (int col = 0; col < sprites::kCols; col++) cell(col, 10 + r, sprites::kLegs[legs][r][col]);
  }

  // Stubby arms: wave when happy/excited/love, otherwise rest at the sides.
  bool waving = mood_ == Mood::kExcited || mood_ == Mood::kLove ||
                (mood_ == Mood::kHappy && (now / 2000) % 3 == 0);
  bool up = waving && (now / 220) % 2;
  if (!asleep) {
    int leftRow = up ? sprites::kArmRowUp : sprites::kArmRowDown;
    int rightRow = waving && !up ? sprites::kArmRowUp : sprites::kArmRowDown;
    cell(0, leftRow, 'b');
    cell(0, leftRow + 1, 'd');
    cell(sprites::kCols - 1, rightRow, 'b');
    cell(sprites::kCols - 1, rightRow + 1, 'd');
  }

  // Face: eyes follow the gaze (or walking direction), blink, and mirror.
  Eyes eyes = asleep ? Eyes::kSleepy : st.eyes;
  bool canBlink = eyes == Eyes::kOpen || eyes == Eyes::kBig || eyes == Eyes::kAngry;
  if (canBlink && now - blinkStart_ < kBlinkMs) eyes = Eyes::kClosed;
  int gx = walking_ ? facing_ : (int)roundf(lookX_);
  int gy = asleep ? 1 : (int)roundf(lookY_);
  const char* const* e = sprites::kEyes[(int)eyes];
  for (int r = 0; r < 3; r++)
    for (int k = 0; k < 3; k++) {
      cell(sprites::kEyeCol + gx + k, sprites::kEyeRow + gy + r, e[r][k]);
      cell(sprites::kRightEyeCol + gx + (2 - k), sprites::kEyeRow + gy + r, e[r][k]);
    }
  if (!asleep) {
    const char* const* m = sprites::kMouths[(int)st.mouth];
    for (int r = 0; r < 2; r++)
      for (int k = 0; k < 4; k++) cell(sprites::kMouthCol + gx + k, sprites::kMouthRow + r, m[r][k]);
  }
  if (st.cheeks) {
    cell(3, 5, 'p');
    cell(12, 5, 'p');
  }
}

void Pet::drawEffects(GFXcanvas16& c, int headX, int headY, uint32_t now) {
  const MoodStyle& st = mood::style(mood_);
  float secs = now / 1000.0f;
  switch (mood_) {
    case Mood::kLove:
      for (int i = 0; i < 3; i++) {
        float p = fmodf(secs * 0.6f + i / 3.0f, 1.0f);
        heart(c, headX - 26 + i * 22 + sinf(secs * 3 + i) * 3, headY - 4 - p * 30, kHeart);
      }
      break;
    case Mood::kSleepy:
      for (int i = 0; i < 3; i++) {
        float p = fmodf(secs * 0.35f + i / 3.0f, 1.0f);
        c.setTextSize(1 + (i == 1));
        c.setTextColor(0xFFFF);
        c.setCursor(headX + 18 + p * 12, headY + 4 - p * 34);
        c.print('z');
      }
      break;
    case Mood::kHot: {
      float p = fmodf(secs * 0.8f, 1.0f);
      c.fillCircle(headX + 30, headY + 14 + p * 12, 2, kTear);
      c.fillTriangle(headX + 28, headY + 13 + p * 12, headX + 32, headY + 13 + p * 12, headX + 30,
                     headY + 9 + p * 12, kTear);
      break;
    }
    case Mood::kSad: {
      float p = fmodf(secs * 0.5f, 1.0f);
      c.fillRect(headX - 16, headY + 32 + p * 14, 3, 4, kTear);
      break;
    }
    case Mood::kCold:
      if ((now / 120) % 2) {
        c.drawFastVLine(headX - 36, headY + 24, 8, kTear);
        c.drawFastVLine(headX + 35, headY + 24, 8, kTear);
      }
      break;
    case Mood::kAngry:
      for (int i = 0; i < 2; i++) {
        float p = fmodf(secs * 0.9f + i * 0.5f, 1.0f);
        c.fillCircle(headX + (i ? 18 : -18), headY - 2 - p * 14, 3 - p * 2, kSteam);
      }
      break;
    default:
      break;
  }
  if (st.mark) {
    int bob = ((now / 300) % 2) * 2;
    c.setTextSize(2);
    c.setTextColor(0x0000);
    c.setCursor(headX + 23, headY - 15 - bob);
    c.print(st.mark);
    c.setTextColor(0xFFE0);
    c.setCursor(headX + 22, headY - 16 - bob);
    c.print(st.mark);
    c.setTextSize(1);
  }
}

void Pet::drawBubble(GFXcanvas16& c, int headX) {
  constexpr int kCharsPerLine = 19, kMaxLines = 5, kLineH = 10;
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
  int h = n * kLineH + 8, y = 3;
  int tailX = constrain(headX, 14, c.width() - 14);
  c.fillRoundRect(2, y, c.width() - 4, h, 6, kWhite);
  c.fillTriangle(tailX - 5, y + h - 1, tailX + 5, y + h - 1, tailX, y + h + 6, kWhite);
  c.setTextSize(1);
  c.setTextColor(0x0000);
  for (int i = 0; i < n; i++) {
    c.setCursor(7, y + 5 + i * kLineH);
    c.print(lines[i]);
  }
}
