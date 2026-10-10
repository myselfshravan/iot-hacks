#include "wavess.h"

namespace wavess {
namespace {

// Timings measured from the WAVESS remote (us).
constexpr uint16_t kHdrMark = 9025, kHdrSpace = 4530, kBitMark = 530;
constexpr uint16_t kOneSpace = 1660, kZeroSpace = 595;
constexpr uint16_t kBits = 40;
constexpr uint16_t kFreqKhz = 38;

// Nibbles that never changed in any capture.
constexpr uint8_t kConstNibbles[10] = {0, 0, 0, 1, 0, 0, 0, 1, 12, 0};

}  // namespace

// Layout (nibbles, LSB first): N0 = power<<3 | mode, N2 = temp - 15,
// N4 = swing<<3 | fan; the rest are constants.
uint64_t encode(const State& s) {
  uint8_t n[10];
  memcpy(n, kConstNibbles, sizeof(n));
  n[0] = (s.power ? 8 : 0) | s.mode;
  n[2] = constrain(s.temp, kMinTemp, kMaxTemp) - 15;
  n[4] = (s.swing ? 8 : 0) | s.fan;
  uint64_t bits = 0;
  for (int i = 0; i < 10; i++) bits |= (uint64_t)(n[i] & 0xF) << (4 * i);
  return bits;  // bit 0 is transmitted first
}

bool decode(uint64_t bits, State* out) {
  uint8_t n[10];
  for (int i = 0; i < 10; i++) n[i] = (bits >> (4 * i)) & 0xF;
  if (n[3] != 1 || n[7] != 1 || n[8] != 12) return false;
  if ((n[0] & 7) >= kModeCount || n[2] < 1 || n[2] > 15) return false;
  out->power = n[0] & 8;
  out->mode = n[0] & 7;
  out->temp = n[2] + 15;
  out->fan = n[4] & 3;
  out->swing = n[4] & 8;
  return true;
}

bool fromRaw(const decode_results& raw, uint64_t* bits) {
  if (raw.rawlen < 3 + 2 * kBits) return false;
  uint32_t hdrMark = raw.rawbuf[1] * kRawTick, hdrSpace = raw.rawbuf[2] * kRawTick;
  if (hdrMark < 7000 || hdrMark > 11000 || hdrSpace < 3500 || hdrSpace > 5500) return false;
  uint64_t out = 0;
  for (int i = 0; i < kBits; i++) {
    uint32_t space = raw.rawbuf[4 + 2 * i] * kRawTick;
    if (space > (kOneSpace + kZeroSpace) / 2) out |= 1ULL << i;
  }
  *bits = out;
  return true;
}

void send(IRsend& tx, uint64_t bits) {
  tx.sendGeneric(kHdrMark, kHdrSpace, kBitMark, kOneSpace, kBitMark, kZeroSpace,
                 kBitMark, 20000, bits, kBits, kFreqKhz, /*MSBfirst=*/false,
                 /*repeat=*/0, /*dutycycle=*/50);
}

}  // namespace wavess
