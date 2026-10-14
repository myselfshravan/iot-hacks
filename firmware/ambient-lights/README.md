# Ambient lights: moved into Dexi

The WS2812B strip behind the monitor is now driven by Dexi's ESP32 (`firmware/dexi/src/light/`),
not a separate WLED board. Wiring: `docs/wiring.md` (Dexi LED strip).

- **Modes** (phone page or `GET /light?mode=mood|rainbow|off&bri=0..255`): mood glow (Dexi's colour, breathing), rainbow, off.
- **Screen sync:** HyperHDR on the Mac → LED device type **udpraw**, host `dexi.local`, port 5568. It overrides the mode while streaming and hands back 2 s after it stops.
- **LED layout:** 102 at 60/m on the 53.5 × 32 cm monitor, starting bottom-right and running anticlockwise: right 19, top 32, left 19, bottom 32. In HyperHDR: input position bottom-right, direction anticlockwise (counter-clockwise).
- **Why not WLED:** WLED would replace Dexi's firmware. Keeping our own lets the strip react to Dexi.

History of the original plan: `../../docs/journal.md` §20.
