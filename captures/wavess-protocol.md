# WAVESS AC remote IR protocol (decoded 2026-10-10)

Raw logs: `raw/wavess-01-power.log`, `raw/wavess-02-temp-sweep.log`,
`raw/wavess-03-mode-fan-swing.log`.

## Framing

NEC-like pulse-distance coding at 38 kHz, **40 data bits**, no repeat frame.

| Part | Measured (us) |
|---|---|
| Header mark / space | ~9025 / ~4530 |
| Bit mark | ~530 |
| `0` space | ~595 |
| `1` space | ~1660 |
| Footer | one bit mark |

IRremoteESP8266 reports it as `UNKNOWN`. Some captures show a 41st "bit": that's
trailing noise past the footer, not data.

## Layout

Bits are numbered in the order received (bit 0 first). Read as ten 4-bit
nibbles, each **LSB first** (bit 0 of the nibble arrives first).

| Nibble | Bits | Meaning |
|---|---|---|
| N0 | 0-3 | bit 3 = **power** (1 on). Bits 0-2 = **mode**: 0 cool, 1 dry, 2 fan, 3 heat, 4 auto |
| N1 | 4-7 | 0 in every capture (timer/sleep? unknown) |
| N2 | 8-11 | **temperature** = N2 + 15 (16-30 °C) |
| N3 | 12-15 | 1 in every capture |
| N4 | 16-19 | bit 3 = **swing** (1 on). Bits 0-1 = **fan**: 0 auto, 1 high, 2 medium, 3 low |
| N5-N6 | 20-27 | 0 in every capture |
| N7 | 28-31 | 1 in every capture |
| N8 | 32-35 | 12 in every capture |
| N9 | 36-39 | 0 in every capture |

**No checksum observed**: single fields change without anything else changing,
so any state can be synthesised.

## Evidence

- Power: ON `N0=8`, OFF `N0=0`, all else identical (2 on/off pairs).
- Temperature: 30 → 16 → 30 sweep gives N2 = 15 … 1, one step per press.
- Mode button from Cool: N0 = 9 (dry, droplet icon), 10 (fan), 11 (heat, sun), 12 (auto), 8 (cool).
- Fan button from Low: N4 low bits 3 → 0 (auto) → 1 (high) → 2 (medium) → 3 (low).
- Swing button: N4 = 11 → 3 → 11 (bit 3 toggles).

## Timer / Sleep (capture 2026-10-10, `raw/wavess-04-timer-sleep.log`, hypotheses)

Presses: Timer ×2, Time+ ×3, Time− ×3, Sleep ×2, T cool ×2, T warm ×2.
Only 9 frames arrived; nibbles shown are N0 N2 N4 N5 N7.

| Frame | N0 | N5 | N7 | Likely button |
|---|---|---|---|---|
| 0x0C101B1F08 | 8 (on) | 1 | 1 | Timer #1 → "timer off" |
| 0x0C102B1F00 | 0 (off) | 2 | 1 | Timer #2 → "timer on" |
| 0x0C202B1F00 / 302 / 402 | 0 | 2 | 2, 3, 4 | Time + ×3 |
| 0x0C302B1F00 / 202 / 102 | 0 | 2 | 3, 2, 1 | Time − ×3 |
| 0x0C808B1F08 | 8 | 8 | 8 | Sleep #1 |
| 0x0C100B1F08 | 8 | 0 | 1 | Sleep #2 (back to normal) |

- **N7 is the timer hours (1-8)**, not a constant. Time + / Time − step it by 1. It reads 8 in the Sleep frame.
- **N5 is the timer/sleep mode:**
  - 0 = no timer.
  - 1 = "timer off": the AC turns off after N7 hours.
  - 2 = "timer on": the AC turns on after N7 hours.
  - 8 = sleep.
  - The Timer button cycles 0 → 1 → 2 → 0 (confirmed by the user). Time ± go 1 ↔ 8 h.
- The "timer on" frame (N5 = 2) clears the power bit while the AC keeps running, so decoders ignore power in N5 = 2 frames.
- **T cool / T warm:** these buttons do nothing on the remote's display either, so they are ignored.
- Kelvin's encoder reproduces the Timer #1 frame (0x0C101B1F08) and the Sleep #1 frame (0x0C808B1F08) bit-exact.
