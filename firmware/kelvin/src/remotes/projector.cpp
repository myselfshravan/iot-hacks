#include "projector.h"

namespace projector {
namespace {

constexpr Button kButtons[] = {
    {"power", 0x61D650AF}, {"settings", 0x61D6F807}, {"s", 0x61D6D02F},
    {"up", 0x61D6AA55},    {"down", 0x61D618E7},     {"left", 0x61D6A857},
    {"right", 0x61D69A65}, {"ok", 0x61D628D7},       {"back", 0x61D6728D},
    {"home", 0x61D6CA35},  {"volup", 0x61D6708F},    {"voldown", 0x61D612ED},
};

}  // namespace

const Button* find(const String& id) {
  for (const auto& b : kButtons)
    if (id == b.id) return &b;
  return nullptr;
}

void send(IRsend& tx, const Button& b) { tx.sendNEC(b.code, 32); }

}  // namespace projector
