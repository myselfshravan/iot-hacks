# Hardware: boards, pins, parts

## Boards
| Board | Role | Notes |
|---|---|---|
| ESP32 #1 (NodeMCU 30-pin, CP2102) | IR hub (Kelvin AC remote + projector) | Too wide for the breadboard: only column j is reachable (`wiring.md`) |
| ESP32 #2 | Ambient lights | |
| ESP32 #3 | Desk pet | Still to buy |
| Arduino Uno R3 | Bench only | No Wi-Fi. Use it for quick 5 V sensor or logic tests, not for products |

One ESP32 can drive every IR device in a room. The same LED sends NEC to the projector and WAVESS to the AC. Several LEDs wired in parallel can cover different directions.

## ESP32 #1 pin map (IR hub)
| GPIO | Use |
|---|---|
| 2 | Onboard blue LED, used as the status light |
| 14 | IR receiver OUT |
| 27 | IR receiver VCC (driven HIGH) |
| 26 | IR TX → 220R → 2N2222A base |
| 25 | DHT data |
| 33 | DHT VCC (driven HIGH) |
| VIN (5 V) | IR LED supply, via 220R |
| Free (column j) | 13, 32; 12 must be LOW at boot; 34, 35, 36, 39 are input-only |

## Names
- **IR hub**: ESP32 #1, the generic IR remote replicator (`irhub.local`).
- **Kelvin**: the AC remote app on the IR hub (page `/`, Alexa "Room AC"). The projector page is `/projector`.
- **Lexi**: the AI desk pet (ESP32 #3). Alternative name Dexy.
- Ambient lights (ESP32 #2): no name yet.

## Parts on hand
- 2× ESP32
- Arduino Uno R3
- Breadboards
- 1.8" ST7735 TFT
- DHT sensor
- VS1838B receiver
- IR LEDs
- 2N2222A transistors
- Resistors
- Jumper wires

Orders and the wishlist are in `to-buy.md`. The full inventory history is in `journal.md` §4.

## Arduino-side extras (on hand, unused)
- **HW-130 motor driver shield.** L293D-based shield for the Uno, drives DC motors and steppers.
- **Bluetooth module, 6 pins:** EN, VCC, GND, TXD, RXD, STATE. Label: "level 3.3 V, power 3.6-6 V". Most likely an HC-05, which is classic Bluetooth serial (SPP). The ESP32 has Bluetooth built in, so it's only useful with the Uno.
- **Small board with TX and RX LEDs, a blue "3362" trimpot and a 3-pin header.** Not identified yet (the 3362 is a 3-pin adjustable resistor). Ask for a photo or the board's markings before using it.
