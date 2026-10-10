// Dexi the pixel cat: behaviour (wander, hop, sleep, shiver...), animation,
// mood effects and the speech bubble. Takes high-level orders (mood, say,
// look, hop) and fills in everything else by itself.
#pragma once

#include <Adafruit_GFX.h>

#include "mood.h"

class Pet {
 public:
  // Sets the mood; after `holdMs` (0 = forever) it returns to the base mood.
  void setMood(Mood m, uint32_t holdMs = 0);
  Mood mood() const { return mood_; }
  Mood baseMood() const { return base_; }

  void say(const String& text, uint32_t ms);
  const String& saying() const { return sayText_; }

  // Points the eyes (-1..1 each axis) for `ms`.
  void lookAt(float x, float y, uint32_t ms);
  void hop();

  void update(uint32_t now);
  void draw(GFXcanvas16& c, uint32_t now);

 private:
  void decide(uint32_t now);
  void drawCat(GFXcanvas16& c, int x0, int y0, uint32_t now);
  void drawEffects(GFXcanvas16& c, int headX, int headY, uint32_t now);
  void drawBubble(GFXcanvas16& c, int headX);

  Mood mood_ = Mood::kNeutral, base_ = Mood::kNeutral;
  uint32_t moodUntil_ = 0;

  String sayText_;
  uint32_t sayUntil_ = 0;

  float x_ = 64, targetX_ = 64;
  int facing_ = 1;  // +1 right, -1 left
  bool walking_ = false;
  uint32_t nextDecision_ = 0, hopStart_ = 0, lastUpdate_ = 0;

  float lookX_ = 0, lookY_ = 0;
  uint32_t holdLookUntil_ = 0, nextGlance_ = 0;
  uint32_t blinkStart_ = 0, nextBlink_ = 2000;
};
