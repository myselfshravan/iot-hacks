# iot-hacks

Home hardware hacks on ESP32, built by a software engineer from zero electronics experience.

**Latest build: a DIY Ambilight for under ₹1,000.** 102 LEDs around a monitor, one ESP32, firmware written from scratch.

[![Anime fight on the DIY Ambilight](https://img.youtube.com/vi/OhIqszJstaU/hqdefault.jpg)](https://youtu.be/OhIqszJstaU)

▶ [Anime fight](https://youtu.be/OhIqszJstaU) · [Houdini fluid simulation](https://youtube.com/shorts/ALgWMjIhsi8) · [Full build story](https://blog.shravanrevanna.me/i-built-my-own-ambilight-for-under-1k)

## The devices

| Device | What it does | Status |
|---|---|---|
| **[Ambient lights](firmware/ambient-lights/README.md)** | 102 WS2812B LEDs around a 24" monitor, following the screen live over Wi-Fi. A new colour reaches the strip about 75 ms after HyperHDR sends it. Effects from a phone page or the terminal. | Working |
| **[Dexi](firmware/dexi/README.md)** | AI desk pet on a 1.8" TFT: animated face, moods, touch, and an [LLM brain](services/dexi-brain/README.md) with memory. Runs on the same ESP32 as the lights. | Working |
| **IR hub** ([`firmware/ir-hub`](firmware/ir-hub/ir-hub.ino)) | Replicates IR remotes over Wi-Fi. Its main app, **Kelvin**, is a Wi-Fi + Alexa remote for a WAVESS-controlled Daikin FCU. The IR protocol was reverse-engineered from scratch, so Kelvin synthesises any AC state, checks every send, and follows the physical remote. | Working |

## How the Ambilight works

```text
Mac (HyperHDR)  ->  Wi-Fi (UDP)  ->  ESP32  ->  102 WS2812B LEDs
                                       |
                                       +--> Dexi's face on the TFT
```

HyperHDR averages a zone of the screen for each LED and streams the colours. The ESP32 paints them with a 1.9 A power cap, so a small 5 V 2 A adapter is enough. When the stream stops, the strip falls back to its own effect, or to Dexi's mood colour.

## Parts for the Ambilight

| Part | Cost |
|---|---|
| ESP32 30-pin dev board | about ₹420 |
| WS2812B strip, 60 LEDs/m, 2 m | ₹449 |
| 5 V 2 A adapter | any you have |
| 220 Ω resistor | a few rupees |

Wiring: [`docs/wiring.md`](docs/wiring.md). Setup: [`firmware/ambient-lights/README.md`](firmware/ambient-lights/README.md).

## Build it

You need [arduino-cli](https://arduino.github.io/arduino-cli/) with the `esp32:esp32` core. The libraries are listed in [`CLAUDE.md`](CLAUDE.md).

```sh
cp firmware/dexi/secrets.example.h firmware/dexi/secrets.h   # add your Wi-Fi details
make flash APP=dexi        # build + flash over USB
make ota APP=dexi          # later updates over Wi-Fi
make lights DO=start       # open HyperHDR and follow the screen
make lights DO=stop        # quit HyperHDR
make check                 # compile every sketch
```

## Repo map

| Path | What |
|---|---|
| `firmware/` | Production sketches, one folder per device |
| `services/dexi-brain/` | Dexi's Python brain, runs on the Mac |
| `libraries/` | Arduino libraries shared by the firmware |
| `captures/` | Reverse-engineered IR protocols and raw logs |
| `docs/` | Wiring, hardware, parts and build history |
| `experiments/` | Lab notebook of one-off sketches |
| `tools/` | Serial logging, web embedding, `lights.sh` |

Contributor and agent notes are in [`CLAUDE.md`](CLAUDE.md).
