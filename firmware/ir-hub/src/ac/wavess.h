// WAVESS AC IR protocol: state model <-> 40-bit frame.
// Spec and evidence: captures/wavess-protocol.md.
#pragma once

#include <Arduino.h>
#include <IRrecv.h>
#include <IRsend.h>

namespace wavess {

const char* const kModes[] = {"cool", "dry", "fan", "heat", "auto"};
const char* const kFans[] = {"auto", "high", "medium", "low"};
constexpr int kModeCount = 5, kFanCount = 4;
constexpr uint8_t kModeCool = 0, kModeDry = 1;
constexpr uint8_t kMinTemp = 16, kMaxTemp = 30;

struct State {
  bool power = false;
  uint8_t mode = kModeCool;  // index into kModes
  uint8_t temp = 24;         // kMinTemp..kMaxTemp °C
  uint8_t fan = 0;           // index into kFans
  bool swing = true;

  bool operator==(const State& o) const {
    return power == o.power && mode == o.mode && temp == o.temp && fan == o.fan &&
           swing == o.swing;
  }
  bool operator!=(const State& o) const { return !(*this == o); }
};

uint64_t encode(const State& s);

// False if `bits` isn't a WAVESS frame.
bool decode(uint64_t bits, State* out);

// Extracts the 40 data bits from a raw capture; false if it isn't WAVESS-shaped.
bool fromRaw(const decode_results& raw, uint64_t* bits);

void send(IRsend& tx, uint64_t bits);

}  // namespace wavess
