# ESP32 Hacks - Journal (history and original brief)

> Archived 2026-10-10 from the old HANDOFF.md. Current status lives in HANDOFF.md; layout and rules in CLAUDE.md.


---

## 0. Session Snapshot (read first) — last updated 2026-10-08

### Where things stand
- **Working end-to-end:** ESP32 captures IR (VS1838B), replays it (IR LED + 2N2222A), and is a **Wi-Fi projector remote**: phone page at `http://192.168.1.33` / `http://projector.local` (`experiments/07_wifi_remote`, currently flashed). The projector obeyed Settings/arrows/OK/Back sent from the phone.
- **AC control WORKS (2026-10-10):** WAVESS protocol decoded (`captures/wavess-protocol.md`: 40-bit, no checksum, power/mode/temp/fan/swing). `experiments/10_ac_controller` is flashed: phone page `http://ac.local` (192.168.31.162 on Wi-Fi `Sukhada_Home`) synthesises any state, and the FCU obeyed it. It also tracks the physical remote via the receiver. The Alexa path (dimmable "AC": % = °C) is built but untested.
- **Waiting on deliveries** (ordered 2026-10-08, see §4 "On order"): LED strip, power, TFT, 2 more ESP32s, tools, sensors. Next session starts when they arrive.
- **Dropped:** a salvaged-vape display experiment (U2 didn't answer SWD; not reprogrammable without solder). All vape files were deleted on request; don't revive it unless asked.

### Toolchain (all verified on the Mac)
- `arduino-cli` (Homebrew), board `esp32:esp32:esp32`, core 3.3.12. Libraries: IRremoteESP8266 2.9.0, Espalexa 2.7.0.
- Compile/flash: `arduino-cli compile -b esp32:esp32:esp32 experiments/<x>` then `arduino-cli upload -b esp32:esp32:esp32 -p /dev/cu.usbserial-0001 experiments/<x>`.
- Serial log + send commands: `uv run --with pyserial python tools/serial_log.py --out captures/raw/<name>.log --seconds N [--no-reset] [--send "cmd"]`. Logs are kept verbatim in `captures/raw/`.
- VS Code IntelliSense: `python3 tools/vscode_intellisense.py` regenerates `.vscode/c_cpp_properties.json` (re-run after adding a sketch/library).
- `esptool` available via `uvx --from esptool esptool ...`.
- Wi-Fi credentials live in `experiments/07_wifi_remote/secrets.h` (git-ignored; template `secrets.example.h`). ESP32 is 2.4 GHz only.

### Gotchas learned the hard way
- **The 30-pin board is too wide for the breadboard**: pins sit in columns a and i, the body covers b-h, so only column **j** (right side) is reachable. 3V3 is buried; the IR receiver is powered from GPIO 27 driven HIGH. New builds should use **female jumpers straight onto the ESP32 headers** instead of the breadboard.
- If upload fails with `termios.error: (22, 'Invalid argument')`, the USB link is wedged: ask the user to unplug/replug USB.
- The user has **no multimeter yet** (one is on order). Until then, use the ESP32 ADC (`experiments/05_ir_rx_diag`) for electrical checks.
- No soldering iron yet (on order). Modules with loose headers (e.g. INMP441) need soldering.
- macOS blocks reading `~/Downloads`; ask the user to put files in the repo.
- Robocraze product search API that works: `curl -s -A "Mozilla/5.0" -G https://robocraze.com/search/suggest.json --data-urlencode "q=..." --data-urlencode "resources[type]=product"`; single product: `https://robocraze.com/products/<handle>.js` (`available`, `price` in paise).

### ESP32 #1 pin map (AC / IR board)
| GPIO | Use |
|---|---|
| 2 | onboard blue LED |
| 14 | IR receiver OUT |
| 27 | IR receiver VCC (driven HIGH) |
| 26 | IR TX → 220R → 2N2222A base |
| VIN (5 V) | IR LED supply via 220R |
| free & reachable (column j) | 13, 12 (strapping: must be LOW at boot), 25, 33, 32; input-only 35, 34, 39, 36 |

Full hole-by-hole layout: `docs/wiring.md`. Projector codes: `captures/projector-remote.md`.

### Next session (when parts arrive)
1. **Lights (ESP32 #2):** flash WLED, wire strip (5 V adapter → strip directly, common GND, data via 220R from one GPIO, 1000 µF across strip power), set ~88 LEDs and a ~1.8 A current limit, then HyperHDR on the Mac for screen sync. Details §20.
2. **Desk pet (ESP32 #3):** bring up the 1.8" ST7735 TFT, then sprites + mood engine. Details §20b.
3. **AC (ESP32 #1):** whenever the WAVESS remote is at hand, capture Power/temps/modes with `experiments/04_ir_capture` (§9-§11). Add DHT22 + LD2410C once delivered.

---

## 1. Who / What This Is

I am a software engineer and want to learn ESP32/electronics by building something useful at home rather than following disconnected tutorials.

I have a home HVAC setup using Daikin mini-split / FCU units, but the original wired thermostat/controller arrangement was not practical for the existing installation. We removed the wired thermostat control and installed a WAVESS universal AC control system with a handheld WAVESS IR remote.

There are **4 FCUs** in the setup.

The interesting behavior is:

- One WAVESS handheld remote can operate any/all of the 4 FCUs.
- The remote is definitely IR-based.
- If the front of the remote's IR LED is blocked, the command does not reach the AC.
- I pointed a phone camera at the remote LED and saw the IR LED flashing/purple when buttons were pressed, confirming IR transmission.
- Therefore the immediate target is to reverse-engineer the **WAVESS IR protocol**, not assume the original Daikin IR protocol is still being used.

The exact WAVESS controller is believed to be **WAVESS-4000** based on the installed setup/product information discussed previously. Treat this as a working assumption until confirmed from the physical PCB/label.

---

## 2. Main Goal

Build a DIY controller that can do all of this:

1. Listen to commands sent by the physical WAVESS remote.
2. Capture and decode the IR protocol.
3. Reproduce the same IR commands from an ESP32.
4. Connect the ESP32 to Wi-Fi.
5. Expose AC controls to a custom app and/or Home Assistant.
6. Integrate with Alexa through Home Assistant (preferred first architecture).
7. Ideally detect when the physical remote is used and update software state.
8. Potentially measure actual room temperature separately with a sensor.
9. Eventually support all 4 FCUs, ideally as separate logical AC devices if we can isolate IR coverage per FCU.

Desired end state:

```text
                         ┌──────────────┐
                         │    Alexa     │
                         └──────┬───────┘
                                │
                         ┌──────▼───────┐
                         │Home Assistant│
                         └──────┬───────┘
                                │
                              Wi-Fi
                                │
                         ┌──────▼───────┐
                         │     ESP32    │
                         │ IR RX + TX   │
                         └──────┬───────┘
                                │ IR
                         ┌──────▼───────┐
                         │ WAVESS-4000  │
                         └──────┬───────┘
                                │
                                ▼
                               FCU
```

A future custom app should use the same underlying device/API layer, so Alexa is not the only interface.

---

## 3. Important State Model

Do not assume that an IR command means the HVAC has reported its actual physical state.

There are three useful levels of state:

### 3.1 Last commanded state
What the most recent remote command says, for example:

```json
{
  "power": true,
  "mode": "cool",
  "temperature": 23,
  "fan": "auto",
  "swing": true
}
```

### 3.2 Observed remote state
If the ESP32 is listening to the IR traffic and sees the physical remote send a command, update software state from that packet.

Example:

```text
Physical remote -> 24 C
ESP32 sees packet -> decode -> state becomes 24 C
```

### 3.3 Actual room / equipment state
This may require a physical temperature sensor. Do not call commanded temperature actual room temperature.

Potential future hardware:

```text
ESP32
 ├── IR RX
 ├── IR TX
 └── SHT31 / BME280 / similar room sensor
```

---

## 4. Current Hardware Inventory

Everything below has already been purchased and was delivered **6 Oct 2026**.

### ESP32

**1 x ESP32 NodeMCU Dev Board, CP2102, 30-pin**

Product:
- ESP32 NodeMCU 30-Pin WiFi Bluetooth Development Board CP2102
- Product page: https://robocraze.com/products/nodemcu-32-wifi-bluetooth-esp32-development-board30-pin

This is the main development board.

Expected characteristics:
- ESP32-WROOM-32 family module
- Wi-Fi
- Bluetooth/BLE
- CP2102 USB-UART bridge
- 30-pin development board
- 3.3 V logic
- Programmable from a MacBook over USB

### IR receiver

**1 x VS1838B IR Receiving Head**

Product page:
- https://robocraze.com/products/ir-receiving-head-vs1838b-remote-control-receiver

Purpose:
- Listen to the WAVESS remote
- Capture demodulated IR pulses
- Feed the signal into an ESP32 GPIO

Important:
- Verify the exact pinout of the physical part before wiring.
- The module is intended for 38 kHz IR remote signals.
- Power from 3.3 V initially so its output is safe for ESP32 GPIO.

### IR transmitter

**1 x 5 mm IR Transmitter + Receiver Pair**

Product page:
- https://robocraze.com/products/5mm-ir-transmitter-receiver-pair

Use:
- The IR LED from this pair is intended to be used as the ESP32 IR transmitter.
- The included receiver is not the primary receiver for the project; the VS1838B is preferred for capture.

Approximate target:
- 940 nm IR LED

### Transistor

**1 x 2N2222A NPN transistor pack, 5 pieces**

Product page:
- https://robocraze.com/products/2n2222-npn-transistor-pack-of-5

Purpose:
- Drive the IR LED as a switch.
- Do not drive the IR LED directly from an ESP32 GPIO at high current.

Important:
- **Do not assume the transistor pinout.** Check the exact part/manufacturer/package before wiring.

### Resistors

**1 x 220 ohm resistor pack, 10 pieces**

Product page:
- https://robocraze.com/products/220-ohm-resistor-pack-of-10

Purpose:
- Current limiting / prototype circuits.
- The final IR LED driver may use a different resistor value depending on LED and transistor measurements. Do not treat 220 ohm as a hard requirement.

### Breadboard

**1 x 400 tie-point solderless breadboard**

Product page:
- https://robocraze.com/products/half-breadboard

**1 x 170-point mini breadboard**

Product page:
- https://robocraze.com/products/mini-breadboard

### Jumper wires

**1 x 40-piece male-to-male jumper wire pack, 20 cm**

Product page:
- https://robocraze.com/products/m2m-jumper-wires-20cm-40pcs

These are enough for the initial breadboard prototype.

### Purchase total

- Product subtotal after discounts: **Rs 635.55**
- Shipping: **Rs 49.00**
- Total paid: **Rs 684.55**
- Taxes included in total: **Rs 104.42**

Do not spend more money on hardware until the current parts have been tested, unless there is a clear reason.

### Also on hand
- Bluetooth speaker (model not yet recorded). The original ESP32 supports Classic Bluetooth A2DP, so it can stream audio to it.
- Projector + its NEC IR remote (test target, codes captured).

### On order (2026-10-08)

The user said "all ordered" for the list below; confirm each item on arrival.

**Amazon**
- WS2812B strip, 5 V, 60 LEDs/m, **2 m**, black PCB, JST connectors both ends. Data is directional: connect at the end the arrows point away from.
- HLK-LD2410C 24 GHz mmWave presence sensor (UART). The Robocraze C4001 was out of stock.

**Robocraze**
| Item | Purpose |
|---|---|
| 2× ESP32 NodeMCU 30-pin CP2102 (same as current board) | ESP32 #2 lights, ESP32 #3 desk pet |
| 1.8" TFT LCD 128×160, ST7735R, SPI | desk pet screen (the 1.3" OLED was out of stock) |
| NOVEL 5 V 2 A single-jack adapter + 5.5×2.1 mm female DC cable | strip power |
| 1000 µF 25 V electrolytic capacitor | across strip power at the input |
| Jumper set M2M + M2F + F2F | wire modules straight to ESP32 headers |
| MB102 830-point breadboard | strip power junction / prototyping |
| 2× micro-USB data cables | for the new boards |
| Noel 7-in-1 beginner soldering kit | headers, strip joints |
| MetroQ MTQ-111 multimeter | electrical checks |
| INMP441 I2S microphone | pet hearing, music-reactive lights |
| TTP223B touch sensor | pet the pet |
| Passive buzzer module | pet chirps |
| DHT22 temp/humidity | room temperature (AC + pet) |

**Decided against (for now)**
- A level shifter. Robocraze has no 74HCT/AHCT125, and the 74LS125 can't output a proper 5 V high. Run the strip data at 3.3 V over a short wire first; if it flickers, add a TXB0104 (Robocraze) or a 74AHCT125 (Amazon). Don't use BSS138 bi-directional converters for WS2812B (too slow).
- The 12 V adapters (would destroy a 5 V strip), and open-frame mains SMPS units.
- Most of `docs/to-buy.md` (NFC, RS485, NRF24, GPS, servos, etc.) until the three current projects work.

---

## 5. Existing HVAC Setup

Current physical setup:

```text
                WAVESS handheld remote
                         │
                         │ IR
                         ▼
                   WAVESS-4000
                         │
                         ▼
                    Daikin FCU
```

There are 4 FCUs, and the same WAVESS remote can control any of them.

Original wired thermostat/control was removed because the wired control could not be conveniently extended / used together with the universal AC controller in this installation.

### Critical assumption

The FCU is Daikin hardware, but **do not automatically use Daikin IR protocol definitions**. The current receiver/controller path is WAVESS. We need to inspect what the WAVESS remote actually transmits.

---

## 6. Why IR Reverse Engineering Comes First

The physical remote has already been confirmed to transmit IR.

The first technical question is:

> What exact waveform/protocol does the WAVESS remote send for each AC state?

Do not start by modifying the FCU PCB.

Treat the remote + WAVESS system as a black box:

```text
input  = button/state selected on WAVESS remote
output = IR waveform
```

Capture the waveform first. Only after we have a reliable capture should we attempt transmission/replay.

---

## 7. Recommended Software Stack for Phase 1

### Development environment

MacBook + Arduino IDE is acceptable for initial development.

Target board:

```text
ESP32 Dev Module
```

when using Arduino IDE for the generic WROOM-32 board.

### Arduino library

Use **IRremoteESP8266** for IR experimentation because it supports ESP32, IR receive/transmit, raw capture, and many AC protocols.

Repository:
- https://github.com/crankyoldgit/IRremoteESP8266

Useful example:

```text
IRrecvDumpV3
```

The goal is to get a raw capture from the WAVESS remote and preserve it exactly.

---

## 8. Initial Hardware Wiring

### Phase A: ESP32 only

No IR hardware.

```text
MacBook
   │ USB
   ▼
 ESP32
```

Verify:
- Board is detected
- Firmware flashes
- Serial Monitor works
- GPIO test works
- Wi-Fi connection works

### Phase B: IR receiver

Proposed initial wiring, **after verifying the physical VS1838B pinout**:

```text
VS1838B       ESP32
--------      --------
VCC    -----> 3V3
GND    -----> GND
OUT    -----> GPIO 14
```

GPIO 14 is a convenient initial capture pin. It can be changed if there is a conflict.

Do not feed 5 V into an ESP32 GPIO.

### Phase C: IR transmitter

Conceptually:

```text
ESP32 GPIO
    │
  resistor
    │
    ▼
 2N2222A
    │
    ▼
 IR LED 940 nm
    │
   GND
```

Exact wiring depends on the transistor pinout and IR LED characteristics. Verify those before building.

Do not connect the ESP32 directly to the WAVESS PCB yet.

---

## 9. First Milestone

Before any Wi-Fi/Alexa/app work, achieve this:

```text
WAVESS remote
      │
      │ IR
      ▼
 VS1838B
      │
      ▼
   ESP32
      │ USB
      ▼
 MacBook Serial Monitor
```

Press one WAVESS button and see a valid IR capture.

The first button should be **Power**.

Then capture representative AC states:

```text
Power ON
Power OFF
Cool + 23 C
Cool + 24 C
Cool + 25 C
Fan LOW
Fan MEDIUM
Fan HIGH
Fan AUTO
Swing ON
Swing OFF
Mode COOL
Other modes exposed by the remote
```

Record both:
- the exact remote state at capture time
- the exact raw serial output

Store captures in a structured local format, ideally something like:

```text
captures/
  power_on.txt
  power_off.txt
  cool_23.txt
  cool_24.txt
  cool_25.txt
  fan_low.txt
  fan_medium.txt
  fan_high.txt
  fan_auto.txt
  swing_on.txt
  swing_off.txt
```

Also create a `captures/README.md` documenting what each capture represents.

---

## 10. Reverse Engineering Plan

For each captured packet:

1. Determine carrier frequency if possible.
2. Determine whether the library recognizes the protocol.
3. Compare packet lengths across commands.
4. Compare repeated fields.
5. Identify fields that change with temperature.
6. Identify fields that change with mode.
7. Identify fields that change with fan speed.
8. Identify fields that change with swing.
9. Check whether there is a checksum/CRC/integrity byte.
10. Determine whether every transmission contains complete state or just a delta/button event.

Very likely outcome:

```text
packet/state
 ├── power
 ├── mode
 ├── temperature
 ├── fan
 ├── swing
 └── checksum/integrity
```

But this is only a hypothesis. Prove it from captures.

### Very important AC behavior

Do not assume one button equals one small command like a TV remote.

Many AC remotes transmit the complete current state in every packet. We need to determine whether WAVESS behaves that way.

---

## 11. Replay Test

After obtaining a known-good capture:

```text
ESP32
  │
  │ reproduce captured IR
  ▼
WAVESS-4000 receiver
  │
  ▼
FCU
```

The first successful experiment should be:

1. Use physical remote to turn FCU OFF.
2. Capture the ON command.
3. Replay the captured ON packet from ESP32.
4. Verify FCU turns ON.

Then repeat with temperature/state commands.

Do not move to a full production architecture until replay is proven.

---

## 12. Multi-FCU Strategy

Current observation:

> One remote can operate any of the 4 FCUs.

Possible architectures:

### Option A: One ESP32 / wide IR coverage

```text
             ESP32 IR TX
                  │
            IR illumination
          /       |       \\
       FCU1     FCU2      FCU3 ...
```

This may cause multiple FCUs to respond to one command, so it is only acceptable if that is desired.

### Option B: One ESP32 per FCU

```text
ESP32 #1 -> FCU #1
ESP32 #2 -> FCU #2
ESP32 #3 -> FCU #3
ESP32 #4 -> FCU #4
```

This is preferable if each FCU should appear as an independent smart climate device.

### Initial recommendation

Prototype with **one ESP32 + one FCU** first.

Do not buy 3 more ESP32 boards until the protocol and hardware path work.

---

## 13. Wi-Fi Control Architecture

After IR control works, the ESP32 should become a Wi-Fi device.

For a simple prototype, HTTP is fine:

```text
GET /on
GET /off
GET /set?temperature=23
```

For the more scalable system, prefer **MQTT**:

```text
home/ac/bedroom/command
home/ac/bedroom/state
```

Command example:

```json
{
  "power": true,
  "temperature": 23,
  "mode": "cool",
  "fan": "auto",
  "swing": true
}
```

State example:

```json
{
  "power": true,
  "temperature": 23,
  "mode": "cool",
  "fan": "auto",
  "swing": true,
  "source": "remote",
  "updated_at": "..."
}
```

---

## 14. Alexa Integration

Preferred architecture:

```text
Alexa
  │
  ▼
Home Assistant
  │
  ▼
MQTT / ESP32 integration
  │
  ▼
ESP32
  │
  ▼
IR
  │
  ▼
WAVESS
```

Reason:
- Avoid building a custom Alexa Smart Home Skill first.
- Home Assistant can expose a climate entity to Alexa.
- The same device model can later be used by a custom app.

Do not spend time on Alexa until the ESP32 can reliably control the FCU locally.

---

## 15. Custom App Goal

I want the option to use a custom UI instead of relying only on Alexa.

Possible UI:

```text
Bedroom AC

Power: ON
Mode: COOL
Target: 23 C
Fan: AUTO
Swing: ON

Room: 25.8 C      <- future sensor

Last command: 12 sec ago
Source: WAVESS remote
```

The app should ideally show the same state that Home Assistant/Alexa sees.

Given my background, a future stack such as:

```text
Next.js / React
        │
      API
        │
      MQTT
        │
     ESP32
```

is totally acceptable.

Do not over-engineer the backend before the hardware protocol is understood.

---

## 16. Remote State Detection

This is a major desired feature.

If someone uses the physical WAVESS remote:

```text
WAVESS remote
      │
      ├──────────────► WAVESS-4000 / FCU
      │
      └──────────────► ESP32 IR receiver
                              │
                              ▼
                           decode
                              │
                              ▼
                          MQTT/state
                              │
                    ┌─────────┴─────────┐
                    ▼                   ▼
             Home Assistant        Custom App
```

This only works if the ESP32 can physically see the remote's IR transmission.

Therefore the receiver placement matters.

Possible limitation:
- If the remote is used while pointing in a direction where the ESP32's IR receiver cannot see the signal, the state will not be observed.

Possible improvement later:
- Place the receiver close to the FCU so it can see the IR path.
- Use multiple receivers if needed.

---

## 17. Actual Temperature / Sensor State

Commanded AC state and room temperature are different.

Future idea:

```text
ESP32
 ├── WAVESS IR RX
 ├── WAVESS IR TX
 └── room temp/humidity sensor
```

Then the UI can show:

```text
Target: 23 C
Room:   25.8 C
```

This can also support automation such as:

```text
if room_temp > threshold:
    turn on AC
```

Do not implement these automations before the core IR path is stable.

---

## 18. Safety / Hardware Rules

These rules are important:

1. **Do not connect the ESP32 GPIO directly to the WAVESS-4000 PCB.** Start with optical/IR interaction only.
2. **Do not apply 5 V to ESP32 GPIO pins.**
3. Verify receiver module pinout before connecting VCC/GND/OUT.
4. Verify the exact 2N2222A package pinout before wiring the transistor.
5. Use a current-limiting resistor for the IR LED.
6. Avoid opening or altering the live FCU wiring unless absolutely necessary.
7. Test IR transmission with the hardware disconnected from anything dangerous.
8. Treat the FCU mains/power side as off-limits for the first iterations.
9. Keep firmware experiments reversible and document every wiring change.

---

## 19. Suggested Repo Structure

Actual layout as of 2026-10-08:

```text
esp32-hacks/
├── HANDOFF.md
├── .gitignore                  # secrets.h, .DS_Store
├── .vscode/                    # generated by tools/vscode_intellisense.py
├── docs/
│   ├── wiring.md               # ESP32 #1 breadboard, hole by hole
│   └── to-buy.md               # the user's shopping notes (AI-generated wishlist)
├── captures/
│   ├── projector-remote.md     # NEC codes for the test projector remote
│   └── raw/                    # verbatim serial logs, never edited
├── experiments/
│   ├── 01_bringup/             # serial + blink + Wi-Fi scan
│   ├── 04_ir_capture/          # IR dump (use this for WAVESS)
│   ├── 05_ir_rx_diag/          # receiver health check via ADC
│   ├── 06_ir_tx/               # serial-driven NEC send + loopback test
│   └── 07_wifi_remote/         # phone web remote + Alexa (Espalexa), flashed now
└── tools/
    ├── serial_log.py
    └── vscode_intellisense.py
```

Convention: one numbered folder per experiment/project under `experiments/`. New projects (lights, pet) get their own folders, and their own ESP32s.
The originally planned `ac-controller/` split can happen once the WAVESS protocol is understood.

---

## 20. Secondary Project: ESP32 + WLED Ambient Lighting

I also want to try the gaming/ambient-lighting project from Reddit that uses ESP32 + addressable LEDs.

Concept:

```text
PC / screen
    │
    ▼
WLED / Hyperion-style software
    │
  Wi-Fi
    │
    ▼
ESP32
    │
    ▼
WS2812B addressable LED strip
```

Important:
- This is a **secondary project**, on its own board (ESP32 #2). Do not block AC work on it.

Reddit reference supplied by me:
- https://www.reddit.com/r/IndianGaming/s/H2NnhvAEY0

### Plan (agreed 2026-10-07/08)

**Monitor:** LG UltraGear 24" (1920×1080, 165 Hz, VA, ~53 × 30 cm). Strip loop mounted a few cm inside the edges is ~1.45-1.5 m, ≈ **88 LEDs** at 60/m:
right ~14, top ~29, left ~14, bottom ~25 (gap at the stand). Confirm the counts after mounting. Fold the strip 45° at corners rather than cutting.

**Wiring**
- 5 V 2 A adapter → DC female cable → **directly** to the strip's input power wires (not through the breadboard or the ESP32). Red is usually +, black −; confirm the markings before connecting.
- 1000 µF capacitor across +5 V/GND at the strip input (mind polarity: the stripe is −).
- ESP32 GND ↔ strip GND (common ground). ESP32 stays powered from USB.
- Data: ESP32 GPIO (pick a non-strapping output, e.g. 16 or 25 on the new board) → 220 Ω → strip DIN, wire < 15 cm.
- No level shifter initially (see §4 "Decided against").

**Software steps**
1. Flash WLED (easiest: install.wled.me web installer in Chrome; a CLI flash needs WLED's own partition layout, since its app is larger than the Arduino default slot).
2. In WLED set the LED count and the **current limiter to ~1800 mA** (adapter is 2 A).
3. Test colours/effects from the WLED app.
4. Screen sync: **HyperHDR** on the Mac (macOS screen capture) → WLED over UDP. Set the per-side LED counts.
5. Later: a custom Mac capture app → own firmware; music-reactive mode via the INMP441; AC-state glow.

Optional later: HDMI sync for consoles (like a Hue Sync Box) needs an HDMI splitter + USB capture card (~₹1,500).

---

## 20b. Side Project: Desk Pet (ESP32 #3 + 1.8" TFT)

The user wants a colourful desk pet / emoji character on a small screen, driven by a service.

**Hardware:** 1.8" ST7735R TFT, 128×160, SPI (SCK, MOSI, CS, DC, RST; backlight to 3.3 V/5 V). Optional: INMP441 mic (hears claps/music), TTP223B (pet it), passive buzzer (chirps), DHT22 (reacts to room heat).

**Architecture (agreed):**
```text
Service (Mac / later cloud) ──Wi-Fi──► ESP32 ──SPI──► TFT
  decides mood/state, may use Claude        stores sprite frames,
  can push new sprites (full frame ≈ 40 KB) animates locally (works offline)
```
- The ESP32 owns smooth animation (idle bob, blink, reactions). The service sends small commands (`mood: sleepy`, `say: "..."`) and occasionally new art.
- Personality hooks: shivers when the AC goes cold, sweats when it's hot, looks up on IR remote use, sleeps when no presence (LD2410C), startles at claps, purrs on touch.

---

---

## 21. Development Philosophy

I am a software engineer, so explanations can be technical, but I am new to practical electronics/ESP32 work.

When helping me:

- Explain what the circuit is doing, not just what wire goes where.
- Prefer small, testable experiments.
- Give exact pin numbers and verify assumptions.
- Keep a clear separation between known facts and hypotheses.
- Do not jump to Home Assistant/Alexa before local IR control works.
- Preserve raw IR captures exactly.
- Prefer reproducible scripts/firmware over manual GUI-only steps where practical.
- Keep everything in this repo so another agent can continue the work.
- When a hardware detail is unknown, inspect the part/label/datasheet rather than guessing.

---

## 22. Current Status

**Hardware:** Arrived 6 Oct 2026.

**ESP32:** Working (2026-10-07). Flashes from the Mac via `arduino-cli` (`esp32:esp32:esp32`, core 3.3.12, IRremoteESP8266 2.9.0) on `/dev/cu.usbserial-0001`. Serial, GPIO and Wi-Fi scan verified (`experiments/01_bringup`).

**IR capture + transmit bring-up:** Done with a projector remote (NEC). VS1838B capture works, IR LED transmitter works (loopback bit-exact), and the projector obeys codes replayed by the ESP32. Wiring in `docs/wiring.md`, codes in `captures/projector-remote.md`, raw logs in `captures/raw/`. Serial logging/commands: `uv run --with pyserial python tools/serial_log.py --help`. No multimeter on hand; use `experiments/05_ir_rx_diag` (ESP32 ADC) for electrical checks.

**IR protocol (WAVESS):** Decoded, see `captures/wavess-protocol.md`. Timer/sleep/T-cool/T-warm buttons are not captured yet.

**WAVESS IR:** Confirmed visually/experimentally to be IR.

**WAVESS-4000:** Working assumption from existing installation context; verify physical PCB/label when convenient.

**FCU control:** Currently works through the existing WAVESS remote.

**Wi-Fi control:** Proven with the projector: `experiments/07_wifi_remote` serves a phone remote page and maps buttons to IR. Not yet for the AC.

**Alexa:** Code path exists (Espalexa emulates a Hue bridge; device "Projector" → Power). Untested: the user's Echo is on a different Wi-Fi network.

**Custom app:** Not implemented.

**Remote state tracking:** Not implemented.

**Room temperature sensing:** Not implemented.

**Multi-FCU control:** Not implemented through ESP32.

---

## 23. Immediate Next Task

Tasks 1-3 and the transmitter bring-up are done (see §0 and §22). Remaining, in order:

### Task 4
Run `experiments/04_ir_capture` and capture the **WAVESS Power** command. Log with `tools/serial_log.py` into `captures/raw/`.

### Task 5
Capture several temperatures and modes.

### Task 6
Commit those captures and analysis notes into the repo.

### Task 7
Replay WAVESS captures through the existing transmitter (`experiments/06_ir_tx` has a loopback self-test). Range is ~1 m at 16 mA LED current; two 220 Ω in parallel (~33 mA) is the first upgrade if the WAVESS receiver is farther away.

---

## 24. Definition of Done for V1

V1 should achieve:

```text
[x] ESP32 flashes from MacBook
[x] ESP32 serial logging works
[x] ESP32 connects to Wi-Fi
[x] VS1838B captures WAVESS remote
[x] WAVESS protocol identified or raw waveform understood
[x] ESP32 reproduces at least Power ON/OFF
[x] ESP32 reproduces at least one temperature command
[x] FCU responds correctly to ESP32 IR
[x] ESP32 exposes a simple Wi-Fi API or MQTT interface
[ ] Home Assistant can control the FCU
[ ] Alexa can control the FCU
[ ] Physical remote commands can be observed when the ESP32 receiver sees them
```

---

## 25. Final Product Vision

Ultimately I want this to feel like a real smart AC, even though the underlying FCU is being controlled through the WAVESS universal IR interface.

```text
                   ┌────────────────────┐
                   │      Alexa         │
                   └─────────┬──────────┘
                             │
                   ┌─────────▼──────────┐
                   │  Home Assistant    │
                   └─────────┬──────────┘
                             │
                        MQTT / API
                             │
                ┌────────────▼────────────┐
                │         ESP32           │
                │                         │
                │ Wi-Fi  IR RX  IR TX    │
                │          +              │
                │     Temp Sensor         │
                └────────────┬────────────┘
                             │ IR
                             ▼
                       WAVESS-4000
                             │
                             ▼
                            FCU

                      + Custom App
```

Build this incrementally. The first real objective is simply:

> **Capture one WAVESS IR command with the ESP32 and understand exactly what happened.**

