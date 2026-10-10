// HTTP API for phone + the laptop brain.
//   GET  /                       control page (src/web/control.html)
//   GET  /state                  {"mood","base","say","gesture":{id,presses,press_ms,gap_ms,total_ms},...}
//   GET  /mood?m=happy[&secs=5]  secs=0/absent: becomes the base mood
//   GET  /say?text=hi[&secs=8]
//   GET  /look?x=-1..1&y=-1..1[&secs=3]
//   GET  /hop
//   GET  /think?secs=4           pack's "thinking" animation (the brain is thinking)
//   GET  /pets                   {"active","pets":[{name,kb}],"free_kb"}
//   GET  /pets/use?name=clawd    switch pet ("builtin" = the drawn blob)
//   POST /pets/upload?name=clawd multipart .pak (dexi_brain/petpacks.py)
//   POST /frame?secs=10          multipart file: raw 128x160 RGB565 (little-endian),
//                                40960 bytes, shown instead of the face for secs
#pragma once

#include <WebServer.h>
#include <WifiLink.h>

#include "../display/screen.h"
#include "../pet/pet.h"
#include "../pet/pet_store.h"
#include "../input/touch.h"

namespace web_api {

struct Deps {
  Pet& pet;
  PetStore& pets;
  Touch& touch;
  WifiLink& wifi;
};

void begin(WebServer& server, const Deps& deps);

// Laptop-pushed image currently on screen, or nullptr when the face should draw.
const uint16_t* activeImage(uint32_t now);

}  // namespace web_api
