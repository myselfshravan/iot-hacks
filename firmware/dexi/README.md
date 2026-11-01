# Dexi: AI desk pet

A character on a 1.8" TFT with an animated face, moods and touch. Dexi's ESP32 also drives the [monitor LED strip](../ambient-lights/README.md).

## How it works

```text
dexi-brain (Mac, any OpenAI-compatible LLM)  ->  Wi-Fi (HTTP)  ->  ESP32  ->  TFT, LED strip
  gathers context, decides mood and lines                          animates locally, works offline
```

- **The ESP32** owns the animation, touch, the phone page and the LED strip.
- **The [brain](../../services/dexi-brain/README.md)** knows the time, the weather, whether you're at the Mac and which app is in front. It asks the LLM what Dexi feels and says, and it remembers things across days.
- **Touch** is read by shape: a boop, quick taps, drumming (Dexi gets annoyed) or a long hold (Dexi purrs). Two quick taps show an info card.

## Parts

| Part | Why |
|---|---|
| ESP32 dev board | Runs everything |
| 1.8" ST7735 TFT, 128×160, SPI | Face and animations |
| TTP223 touch pad | You can pet it |

Pins are in `src/config.h`. Wiring is in [`docs/wiring.md`](../../docs/wiring.md). Sensors for later are in [`docs/to-buy.md`](../../docs/to-buy.md).

## Run it

```sh
cp firmware/dexi/secrets.example.h firmware/dexi/secrets.h   # add your Wi-Fi details
make flash APP=dexi
cd services/dexi-brain && cp .env.example .env               # add your LLM API key
uv run dexi-brain
```

Open `http://dexi.local` on your phone for mood, say, look and light controls. The API is in `src/net/web_api.cpp`.
