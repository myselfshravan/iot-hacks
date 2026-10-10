# Breadboard wiring (current, verified 2026-10-07)

400-point half breadboard: rows 1-30, columns a-e | trench | f-j. In each row,
a-e are joined and f-j are joined.

## ESP32 placement

30-pin ESP32 DevKit (CP2102) straddles the trench, USB end at row 1.
Pins sit in columns **a** and **i**; the board body covers b-h, so the only
reachable holes beside the pins are column **j** (right side). Left-side pins
(including 3V3) have no free hole, which is why the receiver is powered from a GPIO.

| Row | a (left pins) | i (right pins) |
|---|---|---|
| 1 | 3V3 | VIN (5 V from USB) |
| 2 | GND | GND |
| 5 | | D14 (IR RX in) |
| 6 | | D27 (IR RX power) |
| 7 | | D26 (IR TX out) |
| 15 | | EN |

## IR receiver (VS1838B), left half

Dome facing outward; facing the dome, legs are OUT, GND, VCC (left to right).

| Leg | Hole | Jumper |
|---|---|---|
| OUT | c20 | e20 -> j5 (D14) |
| GND | c21 | a21 -> j2 (GND) |
| VCC | c22 | a22 -> j6 (D27, driven HIGH = 3.3 V, receiver draws ~1.5 mA) |

Verified: OUT idles at 3126 mV (measured by the ESP32 ADC, `experiments/05_ir_rx_diag`).

## IR transmitter, right half (updated 2026-10-10: 4× LED resistors, LED on leads)

GPIO 26 -> 220R -> 2N2222A base; emitter -> GND; collector -> IR LED cathode;
LED anode -> 4× 220R in parallel (~55 Ω) -> VIN (5 V). LED current ~60 mA, which is
strong enough that the AC obeys without the LED pointing at it.

| Part | Holes |
|---|---|
| 2N2222A (flat face toward you, legs E-B-C left to right) | g26 (E), g27 (B), g28 (C) |
| Base resistor 220R | i22 - i27 |
| LED resistors, 4× 220R in parallel (bodies arch over the transistor) | f24-f29, g24-g29, h24-h29, i24-i29 |
| IR LED long leg / anode | male-female lead: j29 -> LED |
| IR LED short leg / cathode | male-female lead: j28 -> LED |
| Jumper D26 -> base resistor | j7 -> j22 |
| Jumper 5 V -> LED resistors | j1 -> j24 |
| Jumper emitter -> GND | j26 -> e21 (receiver GND row) |

The LED sits on flying leads so it can be aimed at the FCU. Point the receiver
dome roughly toward the LED too, so the echo check ("IR sent ✓") hears it.
Test: a phone camera (often the front one) shows the LED flashing purple while sending.
More range later: a TSAL6200 940 nm LED with a 33 Ω resistor (~100 mA).

## DHT sensor (Kelvin feedback)

3-pin module, labels printed on the board: `+`/VCC, `OUT`/DATA, `-`/GND. The modules
have the pull-up built in; a bare 4-pin sensor needs 10k between DATA and VCC.

| Module pin | Leg in | Jumper to |
|---|---|---|
| `+` / VCC | c25 | a25 -> j9 (D33, driven HIGH = 3.3 V; ~2 mA). Not 5 V: the pull-up would put 5 V on the data pin |
| `OUT` / DATA | c26 | a26 -> j8 (D25) |
| `-` / GND | c27 | a27 -> d21 (receiver GND row) |

Verified 2026-10-10: DHT22 reads ~28.7 °C / 39 % RH.

For "AC reacted" confirmation, put the sensor in the FCU's outlet airflow, using a
long female-female lead. On the desk it only reports room readings.

# Dexi (ESP32 #2): 1.8" ST7735 TFT

Female-female jumpers straight from the TFT header to the ESP32's **left** header
(no breadboard needed). Module pin names vary; match by meaning.

| TFT pin | Also labelled | ESP32 |
|---|---|---|
| VCC | | 3V3 |
| GND | | GND |
| SCK | SCL, CLK | D18 |
| SDA | MOSI, DIN | D23 |
| CS | | D5 |
| A0 | DC, RS | D16 (RX2) |
| RESET | RST | D17 (TX2) |
| LED | BL, BLK | D4 (backlight, PWM dimmable) |

The SD-card pins on the module (SD_CS, MISO, ...) stay unconnected.
