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

## IR transmitter, right half

GPIO 26 -> 220R -> 2N2222A base; emitter -> GND; collector -> IR LED cathode;
LED anode -> 220R -> VIN (5 V). LED current ~16 mA, base current ~12 mA.

| Part | Holes |
|---|---|
| 2N2222A (flat face toward you, legs E-B-C left to right) | g26 (E), g27 (B), g28 (C) |
| Base resistor 220R | i22 - i27 |
| LED resistor 220R | h24 - h29 |
| IR LED (clear) short leg / cathode | f28 |
| IR LED long leg / anode | f29 |
| Jumper D26 -> base resistor | j7 -> j22 |
| Jumper 5 V -> LED resistor | j1 -> j24 |
| Jumper emitter -> GND | j26 -> e21 (receiver GND row) |

Verified: loopback (TX -> own RX) bit-exact, and the projector obeys replayed
codes at ~1 m. Range is limited by the 16 mA LED current; two 220R in parallel
(~33 mA) is the first upgrade if more range is needed.

## DHT sensor (Kelvin feedback)

3-pin module, labels printed on the board: `+`/VCC, `OUT`/DATA, `-`/GND. The modules
have the pull-up built in; a bare 4-pin sensor needs 10k between DATA and VCC.

| Module pin | ESP32 |
|---|---|
| VCC | D33 (driven HIGH = 3.3 V; sensor draws ~2 mA). Not 5 V: the pull-up would put 5 V on the data pin |
| DATA | D25 (column j, female jumper or breadboard row of D25) |
| GND | GND |

For "AC reacted" confirmation, put the sensor in the FCU's outlet airflow, using a
long female-female lead. On the desk it only reports room readings.
