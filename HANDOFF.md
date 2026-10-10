# Handoff: current status (updated 2026-10-11)

Layout, commands and rules are in `CLAUDE.md`. History and the original brief are in `docs/journal.md`.

## Products
| # | Device | Board | State |
|---|---|---|---|
| 1 | **IR hub** (`irhub.local`): **Kelvin** AC remote + projector page | ESP32 #1 | **Fully working:** phone page (one screen), Alexa "Room AC" (replies OK), timer (none / off in / on in, 1-8 h) and sleep, DHT22 live readings. With the 4× 220R LED, the AC obeys without aiming the LED at it. |
| 2 | **Ambient lights** | ESP32 #2 | Plan only (`firmware/ambient-lights/README.md`). Waiting for the LED strip. |
| 3 | **Dexi** (AI desk pet) | ESP32 #2 | Starting: TFT bring-up (`experiments/11_tft_bringup`). Plan in `firmware/dexi/README.md`. |

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
1. Put the board near the AC on a USB charger (battery would last only 2-3 days). Run the sensor lead into the outlet air so "AC reacted ✓" works.
2. Optional: remote access over HTTPS via a Cloudflare Tunnel. This needs an always-on Pi or similar device in the same house.
3. Dexi (ESP32 #2) in progress; ambient lights on the next board. Add the Bangalore Wi-Fi to their `secrets.h`.

Lessons:
- Espalexa sends IR inside its callback and blocks Alexa's state poll, so queue the command and send it from loop().
- Never rewrite the value Alexa just set, or Alexa says "device is malfunctioning".
