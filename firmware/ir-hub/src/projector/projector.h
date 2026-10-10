// Projector remote (side extra): NEC codes from captures/projector-remote.md.
#pragma once

#include <Arduino.h>
#include <IRsend.h>

namespace projector {

struct Button {
  const char* id;
  uint32_t code;
};

// nullptr if `id` isn't a known button.
const Button* find(const String& id);

void send(IRsend& tx, const Button& b);

}  // namespace projector
