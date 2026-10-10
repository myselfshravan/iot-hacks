# Handoff: current status (updated 2026-10-10)

Layout, commands and rules are in `CLAUDE.md`. History and the original brief are in `docs/journal.md`.

## Products
| # | Device | Board | State |
|---|---|---|---|
| 1 | **IR hub** (`irhub.local`): **Kelvin** AC remote + projector page | ESP32 #1 | Working from the phone. The DHT22 reads live. With the 4× 220R LED, the AC obeys without aiming the LED at it. |
| 2 | **Ambient lights** | ESP32 #2 | Plan only (`firmware/ambient-lights/README.md`). Waiting for the LED strip. |
| 3 | **Lexi** (AI desk pet) | ESP32 #3 (not bought yet) | Plan only (`firmware/lexi/README.md`). TFT is on hand. |

## Kelvin (AC remote) features
- Alexa device renamed **"Room AC"**, because "AC" clashes with a Control4 scene.
- **Delivery feedback:**
  - `echo_ok`: the hub's own receiver heard the frame it sent. If not, it resends, up to 3 attempts.
  - `confirm`: the DHT sensor in the outlet air saw the air move ≥1.5 °C within 3 minutes.
- **Status LED (GPIO 2):**
  - solid: online;
  - slow blink: Wi-Fi down;
  - fast flicker: sending.
- **Phone page:**
  - an offline banner;
  - IR and AC-reaction badges;
  - air temperature and humidity;
  - a Projector tab, kept as a side extra (the AC is the main job).
- **Reliability:**
  - Wi-Fi reconnects with backoff;
  - a 30 s watchdog;
  - the AC state persists across reboots;
  - a `/health` endpoint.

## Next steps
1. **Wire the DHT and flash v2:**
   - DHT data goes to GPIO 25 (column j), VCC to GPIO 33 (driven HIGH), GND to GND (see `docs/wiring.md`).
   - Sensor is a DHT22 (white); already set in `climate.h`.
   - Then run `make flash`.
2. **Test plan:**
   - `irhub.local` → IR sent ✓.
   - Cover the LED → IR not seen ✗ after 3 tries.
   - Power off the router → the LED slow-blinks, then recovers on its own.
   - Sensor at the vent, AC turned on → "AC reacted ✓" within about 2 minutes.
3. **Alexa:** put the Echo on Sukhada_Home, say "discover devices", then "set Room AC to 24".
4. **Range:** add a 2nd 220R in parallel with the LED resistor, for about 33 mA.
5. **Capture the remaining buttons:** Timer, Sleep, T cool and T warm (`captures/wavess-protocol.md`).
