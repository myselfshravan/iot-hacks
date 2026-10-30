---
title: "I Built My Own Ambilight for Under ₹1,000, on the Same ESP32 That Runs My AI Desk Pet"
seoTitle: "DIY ESP32 Ambilight for Under ₹1,000 with HyperHDR"
seoDescription: "102 WS2812B LEDs, one ESP32, custom firmware and HyperHDR on a Mac. How I built a monitor Ambilight for under ₹1,000 and beat a 287 ms bug."
tags: esp32, iot, arduino, hardware, diy
---

**TLDR:** I wrapped 102 LEDs around my monitor and drove them from a single ESP32 running firmware I wrote myself. The strip cost ₹449. The whole build came in under ₹1,000. The same ESP32 also runs Dexi, my AI desk pet, and that is exactly what broke it.

%[https://youtu.be/OhIqszJstaU]

Room lights off, anime fight on, and the wall behind my monitor moves with every hit. The screen stops ending at the bezel.

%[https://youtu.be/ALgWMjIhsi8]

That one is a fluid simulation I rendered in Houdini. Fluid is the real test, because the colour has to roll across the edges of the screen instead of flashing.

A Philips Hue sync setup costs ₹25,000 and up. I paid ₹449 for the strip and about ₹420 for the ESP32. I also knew almost nothing about electronics when I started this repo.

## The parts

| Part | Cost |
|---|---|
| ESP32 dev board, 30-pin | about ₹420 |
| WS2812B strip, 60 LEDs/m, 2 m | ₹449 |
| 5 V 2 A adapter | already had one |
| 220 Ω resistor | a few rupees |

WS2812B is the magic part. Every LED has its own tiny chip, so you can set each one to any colour over a single data wire. The monitor is a 24" LG UltraGear, 53.5 × 32 cm, so going all the way round uses 102 LEDs at 60 per metre.

## How it works

```text
Mac (HyperHDR)  ->  Wi-Fi (UDP)  ->  ESP32  ->  102 LEDs
```

**HyperHDR** is free and open source. It captures the screen and gives every LED its own zone of the image. Mine are the outer 5% on the sides and the outer 10% on the top and bottom. Each LED's colour is the average of its zone, so it follows what's near that edge of the screen.

It then streams the colours over UDP, three bytes per LED. No acknowledgements, no retries. A late colour is a useless colour, so a lost packet just gets replaced by the next one.

The ESP32 side is short. Read every waiting packet, keep the last one, paint it:

```cpp
while (int len = udp_.parsePacket()) {
  uint16_t n = min<int>(len / 3, px_.numPixels() - skip_);
  for (uint16_t i = 0; i < n; i++) {
    uint8_t rgb[3];
    udp_.read(rgb, 3);
    px_.setPixelColor(skip_ + i, rgb[0], rgb[1], rgb[2]);
  }
  udp_.flush();
}
```

The obvious choice here is WLED, which is excellent firmware. But WLED replaces whatever is on the board, and my board already had a job.

## Wiring it without frying anything

The strip has three pads: GND, DI (data in) and +5V. "DI" also marks the input end, which matters because my strip has no arrows on it.

```text
Strip GND   ->  ESP32 GND
Strip DI    ->  220 Ω resistor  ->  ESP32 D25
Strip +5V   ->  adapter +5V
Adapter GND ->  ESP32 GND
```

Two things I learned here:

- **The grounds must be joined.** The data signal only means something relative to a shared ground. Without it, the LEDs show garbage.
- **Never power the strip from the ESP32.** A WS2812B can pull about 60 mA at full white. Multiply by 102 and you get over 6 A, three times what my adapter can give.

The ESP32 only sends 3.3 V on its data pin, and WS2812Bs officially want closer to 5 V. Most guides tell you to add a level shifter. I tried without one and it just worked.

For power, the firmware does the sums on every frame. It estimates the current the frame would draw, and if that goes over 1.9 A, it scales the whole frame down. A small adapter can never be overloaded, whatever is on screen.

## The bug that made it feel dead

It worked, and it felt awful.

The colours were right, but they arrived 0.5 to 1.3 seconds late. A punch landed, and the wall reacted after you'd already moved on. About two thirds of the packets never got used at all.

I blamed Wi-Fi first. Then HyperHDR. So I changed a pile of settings together: 60 fps capture, less smoothing, a different gamma, and the strip on its own CPU core. It got *worse*. Sync went seconds late or stuck completely, and with that many changes at once I couldn't tell which one did it. I rolled it all back and did what I should have done first: measure.

Remember that the ESP32 is also Dexi. Dexi's face lives on a 1.8" TFT screen, and the LED code only gets to run between face frames. So I timed one face frame.

**287 milliseconds.**

Dexi's face was being pushed to the screen one row at a time, with a fresh setup for every row. For nearly a third of a second, nothing else on the chip ran, and packets piled up behind it. The LEDs were fine. Wi-Fi was fine. A cute little face was holding the whole thing hostage.

The fix was a few lines: send the whole frame as one block instead of row by row.

```cpp
tft_.startWrite();
tft_.setAddrWindow(0, 0, kWidth, kHeight);
tft_.writePixels(pixels, kWidth * kHeight, true, false);
tft_.endWrite();
```

The loop stall went from about 300 ms to about 30 ms. Now the strip starts changing about **75 ms** after HyperHDR sends a colour. That is the difference between a light that follows the action and a light that's part of it.

## Meet Dexi

If you look at the bottom of the videos, there's a small glowing screen under the monitor. That's Dexi.

Dexi is an AI desk pet with an animated face, moods and a touch sensor. Pet it and it reacts by the shape of the touch: a boop, quick taps, annoyed if you drum on it, a purr if you hold on. Its brain is a small Python service on my Mac. It knows the time, the weather, whether I'm around and which app I'm in. It asks an LLM what Dexi feels and says, and it remembers things across days.

Sharing a board turned out to be a feature. When the screen stream stops, the strip falls back to Dexi's mood, breathing slowly in Dexi's colour. My monitor's backlight now has feelings.

Dexi is getting its own post.

## Living with it

- **On demand.** HyperHDR uses about 30% of my Mac's CPU, so I start it only when I watch something: `make lights DO=start`, then `make lights DO=stop`.
- **A phone page.** Dexi serves one tap control for Screen, Mood, Rainbow, Candle, Solid, Chase, Sparkle and Off. Candle in a dark room is unreasonably nice.
- **Not just monitors.** The firmware doesn't care who sends the colours. Anything that can send UDP can drive the strip, and the zones can map any part of the image to any LED.

## What I'd tell you if you're building this

1. Measure before you change anything.
2. Change one thing at a time.
3. If one chip does two jobs, the slowest job sets the speed of both.

Everything is open source, including firmware, wiring and HyperHDR settings: [github.com/myselfshravan/iot-hacks](https://github.com/myselfshravan/iot-hacks).

Attention is all you need, baby.
