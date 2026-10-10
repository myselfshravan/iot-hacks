// Dexi's animated face. Keeps itself alive on its own (blinks, glances,
// breathing, mood extras like hearts or zzz) and takes high-level orders:
// mood, speech bubble, where to look.
#pragma once

#include <Adafruit_GFX.h>

#include "mood.h"

class Face {
 public:
  // Sets the mood; after `holdMs` (0 = forever) it returns to the base mood.
  void setMood(Mood m, uint32_t holdMs = 0);
  Mood mood() const { return mood_; }
  Mood baseMood() const { return base_; }

  // Shows a speech bubble for `ms`.
  void say(const String& text, uint32_t ms);
  const String& saying() const { return sayText_; }

  // Points the eyes (-1..1 each axis) for `ms`, then idle glancing resumes.
  void lookAt(float x, float y, uint32_t ms);

  void update(uint32_t now);  // advances animation
  void draw(GFXcanvas16& c, uint32_t now);

 private:
  void drawEye(GFXcanvas16& c, int cx, int cy, int side, float blink);
  void drawMouth(GFXcanvas16& c, int cx, int cy);
  void drawExtras(GFXcanvas16& c, int cx, int cy, uint32_t now);
  void drawBubble(GFXcanvas16& c);

  Mood mood_ = Mood::kNeutral, base_ = Mood::kNeutral;
  uint32_t moodUntil_ = 0;
  Look cur_ = mood::look(Mood::kNeutral);

  String sayText_;
  uint32_t sayUntil_ = 0;

  float lookX_ = 0, lookY_ = 0, targetX_ = 0, targetY_ = 0;
  uint32_t holdLookUntil_ = 0, nextGlance_ = 0;

  uint32_t blinkStart_ = 0, nextBlink_ = 1500;
  uint32_t lastUpdate_ = 0;
};
