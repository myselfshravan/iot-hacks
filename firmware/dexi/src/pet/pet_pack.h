// Plays a pet pack (.pak built by services/dexi-brain/dexi_brain/petpacks.py)
// from LittleFS: animated GIF-derived sprites, one animation per mood.
#pragma once

#include <Adafruit_GFX.h>
#include <FS.h>

#include "mood.h"

class PetPack {
 public:
  static constexpr int kSlots = 12;  // moods in Mood order, then "thinking"
  static constexpr int kThinkingSlot = 11;

  bool load(fs::FS& fs, const String& path);  // false if missing or malformed
  void unload();
  bool loaded() const { return file_; }

  int width() const { return w_ * scale_; }
  int height() const { return h_ * scale_; }

  // Draws the animation for `slot` with its bottom-centre at (cx, bottom).
  void draw(GFXcanvas16& c, int slot, int cx, int bottom, uint32_t now);

 private:
  struct Anim {
    uint8_t frames;
    uint16_t frameMs;
    uint32_t table;
  };
  static constexpr int kMaxAnims = 16;

  fs::File file_;
  uint16_t w_ = 0, h_ = 0;
  uint8_t scale_ = 1, animCount_ = 0;
  uint8_t slotAnim_[kSlots] = {};
  Anim anims_[kMaxAnims];
};
