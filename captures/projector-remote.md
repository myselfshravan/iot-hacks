# Projector remote (test remote, NEC protocol)

Used to bring up IR capture and transmit before the WAVESS remote is available.
Captured 2026-10-07 with `experiments/04_ir_capture` / `06_ir_tx` (VS1838B on GPIO 14).

Raw log: `raw/projector-button-map.log` (buttons pressed once each, in the order below; the last two were the S and Settings buttons, not Focus +/-).

| Button | NEC code |
|---|---|
| Power  | `0x61D650AF` |
| Up     | `0x61D6AA55` |
| Down   | `0x61D618E7` |
| Left   | `0x61D6A857` |
| Right  | `0x61D69A65` |
| Vol+   | `0x61D6708F` |
| Vol-   | `0x61D612ED` |
| Back   | `0x61D6728D` |
| Home   | `0x61D6CA35` |
| OK     | `0x61D628D7` |
| S        | `0x61D6D02F` |
| Settings | `0x61D6F807` |

OK was captured separately: `raw/projector-ok-button.log` (the `RX Home` lines at its top are stale serial output from earlier phone-triggered sends).

Notes:
- All codes share the 16-bit address `0x61D6`; the low 16 bits are command + inverted command (standard NEC).
- `NEC (Repeat)` frames in the log are the "button still held" code and carry no command.
- `raw/session-02-capture.log` (address `0xC03F`) is a different remote, not this one.
