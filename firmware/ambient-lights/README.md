# Ambient lights (ESP32 #2): plan

WS2812B strip behind the 24" LG monitor, synced to the screen.

- **Firmware:** WLED, installed with the web flasher at install.wled.me. No custom code at first.
- **Screen sync:** HyperHDR on the Mac, sending to WLED over UDP.
- **LEDs:** about 88 at 60/m (right 14, top 29, left 14, bottom 25). Set the current limit to about 1800 mA.
- **Wiring:**
  - 5 V 2 A adapter straight to the strip, with 1000 µF across it at the input.
  - Common GND with the ESP32.
  - Data runs from a non-strapping GPIO (16 or 25) through 220R, on a wire shorter than 15 cm.
- **Later:** custom firmware if WLED limits us, music-reactive mode, and an AC-state glow.

Full notes are in `../../docs/journal.md` §20.
