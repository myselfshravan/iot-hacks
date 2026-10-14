# esp32-hacks

Home hardware hacks on ESP32. The owner is a software engineer who is new to
electronics, so explain the circuit and give exact pins. Read `HANDOFF.md` first
for the current status.

## Layout
| Path | What |
|---|---|
| `firmware/<product>/` | Production sketches, one folder per device. Each is self-contained. |
| `firmware/ir-hub/` | ESP32 #1: **IR hub**, which replicates IR remotes. Main app: **Kelvin**, the AC remote (`src/ac/`, `src/web/ac.html`). Side app: projector (`src/projector/`, `src/web/projector.html`). Code map at the top of `ir-hub.ino` |
| `firmware/ambient-lights/` | Pointer only: the monitor LED strip now lives in Dexi (`firmware/dexi/src/light/`) |
| `firmware/dexi/` | ESP32 #2: **Dexi**, the AI desk pet on a 1.8" TFT (face engine, touch, HTTP API) + the monitor LED strip |
| `services/dexi-brain/` | Python (uv) brain for Dexi on the Mac: any OpenAI-compatible LLM; config in git-ignored `.env` |
| `libraries/` | Arduino libraries shared by all firmware (`WifiLink`); the Makefile passes `--libraries libraries` |
| `experiments/` | Lab notebook: numbered one-off sketches. Don't edit; copy ideas into `firmware/`. |
| `captures/` | Decoded protocols (`*.md`) + verbatim serial logs (`raw/`, never edit) |
| `docs/` | `hardware.md` (boards, pins, parts), `wiring.md`, `to-buy.md`, `journal.md` (history) |
| `tools/` | `serial_log.py` (log/send over serial), `embed_web.py` (HTML → header), `vscode_intellisense.py`, `lights_watchdog.py` (restarts HyperHDR when sync goes stale; `make lights-watchdog` installs it at login) |

## Commands
- `make flash APP=ir-hub` builds and flashes (`APP=experiments/06_ir_tx` also works).
- `make monitor` and `make log NAME=x SECONDS=60` (log saved to `captures/raw/x.log`).
- `make check` compiles every sketch. Run it before committing.
- `make intellisense` after adding a sketch or library.
- Toolchain: arduino-cli, `esp32:esp32:esp32` core 3.3.12. Libraries: IRremoteESP8266 2.9.0, Espalexa 2.7.0, Adafruit DHT sensor library 1.4.7, Adafruit ST7735 and ST7789 + Adafruit GFX, Adafruit NeoPixel 1.15.5 (LovyanGFX installed but garbles this TFT).
- Port: `/dev/cu.usbserial-0001`.

## Code style
- Product sketches: thin `.ino` (wiring + setup/loop), modules under `src/<area>/` as `.h` + `.cpp`, pins/tunables in `src/config.h`.
- Web UI is real HTML in `src/web/`; never edit the generated `*_html.h` (`make build` regenerates it).

## Rules
- Wi-Fi credentials go in `secrets.h`, which is git-ignored. Copy `secrets.example.h`. The user types the password themselves.
- One serial logger at a time. Run `pgrep -fl serial_log` before starting one.
- If an upload fails with `termios.error 22`, ask the user to replug the USB cable.
- Known facts go in docs and captures. Label anything unverified as a hypothesis.
- Temp files go in the session scratchpad, never in the repo.
- Push after every commit (the user wants local and remote kept in sync).
- API keys go in git-ignored `.env` files. Never print or commit them.
