# Experiments (lab notebook)

These are one-off sketches, kept as a record. Products live in `../firmware/`.

| Sketch | What it proved |
|---|---|
| `01_bringup` | Serial, the blink LED and a Wi-Fi scan all work |
| `04_ir_capture` | Raw IR dump from the VS1838B |
| `05_ir_rx_diag` | Receiver health check, using the ESP32 ADC as a voltmeter |
| `06_ir_tx` | NEC send over serial, plus a loopback test |
| `07_wifi_remote` | Projector phone remote plus Alexa (merged into `firmware/ir-hub`) |
| `10_ac_controller` | First WAVESS AC controller (v1, which became `firmware/ir-hub`) |
| `11_tft_bringup` | 1.8" ST7735R TFT test pattern + flicker-free animated face (Adafruit_ST7735, BLACKTAB, 20 MHz) |
| `12_tft_variants` | Cycles ST7735 init variants to find the right one (BLACKTAB for our Robocraze module) |
