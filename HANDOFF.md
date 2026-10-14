# Handoff: current status (updated 2026-10-11)

Layout, commands and rules are in `CLAUDE.md`. History and the original brief are in `docs/journal.md`.

## Products
| # | Device | Board | State |
|---|---|---|---|
| 1 | **IR hub** (`irhub.local`): **Kelvin** AC remote + projector page | ESP32 #1 | **Fully working:** phone page (one screen), Alexa "Room AC" (replies OK), timer (none / off in / on in, 1-8 h) and sleep, DHT22 live readings. With the 4× 220R LED, the AC obeys without aiming the LED at it. |
| 2 | **Ambient lights** | ESP32 #2 (shared with Dexi) | Firmware done (`firmware/dexi/src/light/`): mood glow, rainbow, HyperHDR udpraw sync on port 5568. Strip wired and verified (rainbow and mood); HyperHDR setup next. |
| 3 | **Dexi** (AI desk pet) | ESP32 #2 | **v3 running** at `dexi.local`:
- Pets: the built-in chibi blob, or the Clawd, Calico and Cloudling packs (the active one is chosen in the console).
- Touch: gesture-aware.
- A day/night world.
- Wireless updates: `make ota APP=dexi`.

Brain: `services/dexi-brain`. Its console at localhost:8765 has a replayable thought log, memory and a pet picker. |

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

## Waiting on
- The **Dexi add-ons order**: see `docs/to-buy.md` (top 3: MAX98357A + speaker, LD2410B, vibration motor). When the parts arrive:
  1. Seat ESP32 #2 across two breadboards so the right-side pins are free.
  2. Wire the parts.
  3. Add a firmware module for each, and have the brain describe each new sensor's events to the LLM, the same way it does for touch.
- The **IR proximity sensor** (FC-51-like) is already on hand, for wave and hover reactions. Needs its pin labels first.

## Next steps
1. Put the board near the AC on a USB charger (battery would last only 2-3 days). Run the sensor lead into the outlet air so "AC reacted ✓" works.
2. Optional: remote access over HTTPS via a Cloudflare Tunnel. This needs an always-on Pi or similar device in the same house.
3. Dexi (ESP32 #2) also drives the monitor LED strip. Strip verified on D25; next, set up HyperHDR screen sync.

Lessons:
- Espalexa sends IR inside its callback and blocks Alexa's state poll, so queue the command and send it from loop().
- Never rewrite the value Alexa just set, or Alexa says "device is malfunctioning".
