// HTTP API for phone + the laptop brain.
//   GET  /                       control page (src/web/control.html)
//   GET  /state                  {"mood","base","say","taps","long_presses",...}
//   GET  /mood?m=happy[&secs=5]  secs=0/absent: becomes the base mood
//   GET  /say?text=hi[&secs=8]
//   GET  /look?x=-1..1&y=-1..1[&secs=3]
//   POST /frame?secs=10          multipart file: raw 128x160 RGB565 (little-endian),
//                                40960 bytes, shown instead of the face for secs
#pragma once

#include <WebServer.h>
#include <WifiLink.h>

#include "../display/screen.h"
#include "../face/face.h"
#include "../input/touch.h"

namespace web_api {

struct Deps {
  Face& face;
  Touch& touch;
  WifiLink& wifi;
};

void begin(WebServer& server, const Deps& deps);

// Laptop-pushed image currently on screen, or nullptr when the face should draw.
const uint16_t* activeImage(uint32_t now);

}  // namespace web_api
