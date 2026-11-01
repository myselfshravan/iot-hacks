# Ambient lights

A DIY Ambilight: 102 WS2812B LEDs around a 24" monitor, following the screen live. Build cost: under ₹1,000.

Watch it: [anime fight](https://youtu.be/OhIqszJstaU) · [Houdini fluid simulation](https://youtube.com/shorts/ALgWMjIhsi8). Full story: [the blog post](https://blog.shravanrevanna.me/i-built-my-own-ambilight-for-under-1k).

There's no firmware in this folder. The strip is driven by Dexi's ESP32 (`firmware/dexi/src/light/`), so it can react to Dexi. Wiring: [`docs/wiring.md`](../../docs/wiring.md) (Dexi LED strip).

- **Modes** (phone page, `make lights DO=...`, or `GET /light?mode=..&color=rrggbb&sync=on|off&bri=0..255`):
  - mood: Dexi's colour, breathing;
  - rainbow;
  - solid, chase (a comet running round the monitor), sparkle: all in the chosen colour;
  - candle: warm flicker;
  - off: total off, the stream is ignored too.
- **Screen vs effects:** choosing an effect turns screen sync off (HyperHDR keeps running, Dexi ignores it). `screen` turns it back on, with mood as the fallback.
- **Screen sync:** HyperHDR on the Mac → LED device type **udpraw**, host `dexi.local`, port 5568. It overrides the mode while streaming and hands back 2 s after it stops.
- **LED layout:** 103 on the strip. LED 0 sits outside the monitor and is kept dark (`kLedSkip`). The visible 102 run from the bottom-right, anticlockwise: right 19, top 32, left 19, bottom 32. HyperHDR sends 102. In HyperHDR: input position bottom-right, direction anticlockwise (counter-clockwise).
- **Why not WLED:** WLED would replace Dexi's firmware. Keeping our own lets the strip react to Dexi.

History of the original plan: `../../docs/journal.md` §20.

**HyperHDR 22 setup (done 2026-10-14):**
- Device `udpraw` → `dexi.local`:5568. Use the hostname, because the IP changes after Wi-Fi drops.
- Capture: `systemInstanceEnable` on, USB capture off, system capture device `Display id: 2` (the LG; `reorder_displays` does not pick the screen). Restart HyperHDR if the monitor was off when it started.
- 102 LEDs written as an explicit list: right bottom→top, top right→left, left top→bottom, bottom left→right. Capture depth: 5% on the sides, 10% top and bottom.
- Settings: 30 fps capture, smoothing 80 ms at 50 Hz, gamma 1.5, black-bar detection off. Screen sync runs at full brightness under the 1.9 A cap.
- Tried and reverted 2026-10-14: 60 fps capture, 40 ms smoothing, gamma 1.0 with boosted saturation, and a core-0 strip task. Together they made sync seconds late or stuck, and the culprit was not isolated. Change one setting at a time.
- macOS system capture on, display `auto`.
- Opening the Classic layout editor and saving regenerates the list from its own fields, so re-check the direction after doing that.

**Latency, measured 2026-10-14** (a colour pushed through HyperHDR, then Dexi's `/light` average polled):
- **Root cause of the lag:** `Screen::present` used `drawRGBBitmap`, which took **287 ms** a frame. The strip only runs between frames, so colours arrived 0.5–1.3 s late and about two thirds of packets were dropped.
- **Fix:** one block `writePixels`. The loop gap is now about 30 ms, and the strip starts changing about 75 ms after a colour change.
- Smoothing adds the fade on top of that.
- `/light` reports `loop_gap_ms` (worst loop stall over the last 2 s) and `queued` (most packets drained at once) to catch regressions.
- **HyperHDR on/off:** `make lights DO=start` opens it and follows the screen; `make lights DO=stop` quits it (it uses ~30% CPU). The strip keeps its own effect while it's off.
- **Sync stuck?** Run `make lights DO=restart`. It restarts HyperHDR and prints Dexi's `/light`; a rising `packets` count means sync is back.
