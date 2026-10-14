# Ambient lights: moved into Dexi

The WS2812B strip behind the monitor is now driven by Dexi's ESP32 (`firmware/dexi/src/light/`),
not a separate WLED board. Wiring: `docs/wiring.md` (Dexi LED strip).

- **Modes** (phone page or `GET /light?mode=mood|rainbow|off&bri=0..255`): mood glow (Dexi's colour, breathing), rainbow, off.
- **Screen sync:** HyperHDR on the Mac → LED device type **udpraw**, host `dexi.local`, port 5568. It overrides the mode while streaming and hands back 2 s after it stops.
- **LED layout:** 103 at 60/m on the 53.5 × 32 cm monitor, starting bottom-right and running anticlockwise: right 20, top 32, left 19, bottom 32. In HyperHDR: input position bottom-right, direction anticlockwise (counter-clockwise).
- **Why not WLED:** WLED would replace Dexi's firmware. Keeping our own lets the strip react to Dexi.

History of the original plan: `../../docs/journal.md` §20.

**HyperHDR 22 setup (done 2026-10-14):**
- Device `udpraw` → `dexi.local`:5568. Use the hostname, because the IP changes after Wi-Fi drops.
- Capture: `systemInstanceEnable` on, USB capture off, `reorder_displays` 1 so it grabs the LG (display 2). Restart HyperHDR if the monitor was off when it started.
- 103 LEDs written as an explicit list: right bottom→top, top right→left, left top→bottom, bottom left→right.
- macOS system capture on, display `auto`.
- Opening the Classic layout editor and saving regenerates the list from its own fields, so re-check the direction after doing that.
