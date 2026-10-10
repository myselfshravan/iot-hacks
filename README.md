# iot-hacks

ESP32 home hacks, built from zero electronics experience.

| Device | What it does | Status |
|---|---|---|
| **Kelvin** (`firmware/kelvin`) | Wi-Fi + Alexa remote for a WAVESS-controlled Daikin FCU. The IR protocol was reverse-engineered from scratch, so Kelvin synthesises any AC state, checks every send, and follows the physical remote. | Working |
| **Ambient lights** (`firmware/ambient-lights`) | WS2812B strip behind the monitor, synced to the screen | Planned |
| **Lexi** (`firmware/lexi`) | AI desk pet on a 1.8" TFT | Planned |

- Protocol write-ups are in `captures/`.
- Wiring and hardware notes are in `docs/`.
- Contributor and agent notes are in `CLAUDE.md`.

```sh
make flash APP=kelvin   # build + flash
make check              # compile every sketch
```
