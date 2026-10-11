# To buy

## Dexi add-ons (researched 2026-10-11)

Prices are in ₹, checked 2026-10-11. **Best** is the cheaper in-stock option.

### Top 3: start here
| Part | What Dexi gains | Robocraze | FlyRobo | Best |
|---|---|---|---|---|
| I2S amp **MAX98357A** | A voice: chirps and spoken AI lines | [₹190](https://robocraze.com/products/smartelex-max98357a-i2s-audio-breakout-amplifier-for-raspberry-pi-and-microcontrollers) | [₹323](https://www.flyrobo.in/max98357-i2s-3w-class-d-audio-amplifier-module) | RC |
| Speaker **4 Ω 3 W, 40 mm** (pairs with the amp) | Loud, clear sound | only 0.5 W: [₹53](https://robocraze.com/products/8-ohm-0-25w-speaker) | [₹224](https://www.flyrobo.in/4r-3w-40mm-mini-power-amplifier-speaker) | FR |
| mmWave presence **LD2410B** | Knows you're at the desk even when still | not stocked (C4001 [₹1,575](https://robocraze.com/products/mmwave-c4001-24ghz-human-presence-detection-sensor) is overkill) | [₹548](https://www.flyrobo.in/ld2410b-human-presence-radar-module) | FR |
| Coin **vibration motor** (driven by a 2N2222 you have) | Purrs in your hand | [₹25](https://robocraze.com/products/coin-type-micro-vibration-motor) | [₹26](https://www.flyrobo.in/round-micro-vibration-motor) | either |

### Nice to have
| Part | What Dexi gains | Robocraze | FlyRobo | Best |
|---|---|---|---|---|
| **INMP441** I2S mic (needs soldering) | Hears claps, music and voice | [₹135](https://robocraze.com/products/inmp441-mems-high-precision-omnidirectional-microphone-module-i2s) | [₹438](https://www.flyrobo.in/inmp441-mems-high-precision-omnidirectional-microphone-module-i2s-interface-supports-esp32) | RC |
| **MPU6050** accel/gyro | Knows when it's picked up, shaken or tilted | [₹151](https://robocraze.com/products/mpu-6050-triple-axis-accelerometer-gyroscope-module) | [₹218](https://www.flyrobo.in/mpu_6050_6dof_3_axis_gyro_with_accelerometer_sensor_module) | RC |
| **BH1750** light sensor | Dims at night, reacts to lights off | [₹100](https://robocraze.com/products/gy-302-bh1750-light-intensity-module) | [₹112](https://www.flyrobo.in/gy-30-bh1750-digital-light-intensity-sensor-module) | RC |
| **DHT22** temperature/humidity | Its own room temperature (Kelvin's is in another city) | [₹104](https://robocraze.com/products/dht22-humidity-and-temperature-sensor-module-am2302) | [₹105](https://www.flyrobo.in/dht22-digital-temperature-and-humidity-sensor-module) | either |
| **WS2812 12-LED ring** | Glow that matches its mood | 8-LED: [₹41](https://robocraze.com/products/ws2812-8-bit-rgb-led-round) | 12-LED: [₹74](https://www.flyrobo.in/12bit-ws2812-5050-rgb-led-built-full-color-driving-lights-circular-development-board) | FR |
| **1.28" round GC9A01** 240×240 | Round EMO-style face, sharper | [₹449, out of stock](https://robocraze.com/products/1-28-inch-round-tft-display-module-240x240-hd-ips-full-color-screen) | not found | wait for RC |
| **SG90** micro servo | A wagging tail or tilting head | [₹105](https://robocraze.com/products/sg90-micro-servo-motor) | [₹123](https://www.flyrobo.in/sg90-9g-micro-pro-servo) | RC |
| **TP4056** charger (Type-C) + battery | Cordless | [₹19](https://robocraze.com/products/tp4056-battery-charger-c-type-module-with-protection-1) + 3.7 V 1000 mAh [₹185](https://robocraze.com/products/3-7v-1000mah-li-ion-battery) | [₹19](https://www.flyrobo.in/tp4056-1a-li-ion-lithium-battery-charging-module-with-current-protection-type-c) + 500 mAh [₹276](https://www.flyrobo.in/3.7v-500mah-lipo-rechargeable-battery-lithium-polymer) | RC |

### Also needed
| Part | Why | Robocraze | FlyRobo |
|---|---|---|---|
| **ESP32 30-pin (CP2102)** | ESP32 #3 for the ambient lights (Dexi took #2) | [₹420](https://robocraze.com/products/nodemcu-32-wifi-bluetooth-esp32-development-board30-pin) | [₹438](https://www.flyrobo.in/esp32-node-mcu-wifi-bluetooth-development-module-30-pin) |

**Already on hand:**
- The IR proximity sensor (FC-51-like), which gives wave and hover reactions.
- A spare 2N2222A transistor, to drive the vibration motor.

**Which store:**
- **Top 4 only:** buy everything from FlyRobo, about ₹1,121 in one order. Robocraze doesn't stock the LD2410 or a 3 W speaker.
- **Top 4 plus nice-to-haves:** split the order. FlyRobo for the radar, speaker and LED ring; Robocraze for the rest, which is about ₹550 cheaper. The INMP441 alone is ₹135 vs ₹438.
- **Shipping:** not checked. Compare final cart totals.

**Pins:** to use more than about 2 of these, seat Dexi's ESP32 across two breadboards pushed side by side, so both rows of pins are reachable (see `wiring.md`).

---

## Earlier wishlist notes (pasted, 2026-10)

Haha, now you have crossed the dangerous line. Once the ESP32 successfully reproduced the WAVESS IR packet, you are basically holding a tiny Wi-Fi computer with a stupid number of possible peripherals.

Your **ESP32-WROOM-32** is surprisingly capable for this. It has Wi-Fi, Bluetooth/BLE, I2S, SPI, I2C, PWM, capacitive touch, ADC/DAC, CAN-compatible TWAI, pulse counters and a dedicated infrared peripheral. It also has up to 32 GPIOs, although some pins have boot/strapping constraints. [Espressif Documentation](https://documentation.espressif.com/esp32-wroom-32_datasheet_en.html?utm_source=chatgpt.com)

So rather than buying another development board, I would turn your current one into a **modular home-hacking node**.

INMP441 I2S MEMS Microphone₹135·Robocraze

MPU6050 Accelerometer + Gyro₹154·Robocraze

PN532 NFC RFID Module₹244·Robokits India

## The stuff I'd seriously consider

### 1. 24 GHz mmWave radar 🔥

This is probably my **#1 recommendation**.

[DFRobot C4001 24 GHz mmWave Presence Sensor](https://robocraze.com/products/mmwave-c4001-24ghz-human-presence-detection-sensor) is currently listed at ₹1,569. It provides presence/motion detection, distance and velocity information, communicates over UART, and advertises presence detection up to 16 m and motion detection up to 25 m. [Robocraze](https://robocraze.com/products/mmwave-c4001-24ghz-human-presence-detection-sensor?utm_source=chatgpt.com)

This lets you build:

```text
Someone enters room
       ↓
ESP32 detects presence
       ↓
Check temperature + humidity
       ↓
AC already on?
       │
       ├── no → turn AC on
       └── yes → leave it alone
```

The really cool part is combining it with your existing IR system:

```text
                ESP32
           ┌─────┼──────┐
           │     │      │
          IR    Radar  Temp
           │     │      │
           ▼     ▼      ▼
          AC   Human   Room
```

Then your app can say:

```text
Bedroom
● Occupied
● 26.4°C
● AC 23°C
● Fan Auto
● Occupied for 42 min
```

This is **much better than PIR** because radar can detect motion/presence without relying on visible light or a camera. Robocraze also has the cheaper RCWL-0516 radar module for ₹35, though its behavior is much cruder than the 24 GHz C4001. [Robocraze](https://robocraze.com/products/microwave-radar-human-body-sensor?utm_source=chatgpt.com)

---

### 2. INMP441 digital microphone 🎙️

This tiny thing is ridiculous for ₹148.

[INMP441 I2S microphone](https://robocraze.com/products/inmp441-mems-high-precision-omnidirectional-microphone-module-i2s)

It gives the ESP32 actual digital audio over I2S, rather than a crude analog sound-level signal. Robocraze lists 24-bit I2S, roughly 60 Hz to 15 kHz response and 61 dBA SNR. [Robocraze](https://robocraze.com/products/inmp441-mems-high-precision-omnidirectional-microphone-module-i2s?utm_source=chatgpt.com)

You can build:

**Sound-reactive lights**

```text
Music
 ↓
Microphone
 ↓
ESP32 FFT
 ↓
Bass / mids / treble
 ↓
WS2812B
```

Or:

**Clap-controlled automation**

```text
👏
 ↓
ESP32
 ↓
detect pattern
 ↓
lights ON
```

Or get more ambitious:

```text
Microphone
   ↓
audio features
   ↓
ESP32
   ↓
Wi-Fi
   ↓
LLM/server
   ↓
interpret event
```

The classic WROOM-32 supports I2S, so this is a natural fit. [Espressif Documentation](https://documentation.espressif.com/esp32-wroom-32_datasheet_en.html?utm_source=chatgpt.com)

---

### 3. PN532 NFC reader 📱

[PN532 NFC RFID V3](https://robocraze.com/products/nfc-near-field-communication-rf-id-v3-module)

This one is **very fun for a smart home**.

Robocraze lists I2C, SPI and UART support, roughly 5 to 7 cm reading distance, and Android/NFC support. [Robocraze](https://robocraze.com/products/nfc-near-field-communication-rf-id-v3-module)

Imagine putting NFC tags around your house:

```text
🏠 Bedside
     ↓ tap phone
Sleep mode

🚪 Entrance
     ↓ tap
Away mode

🛋️ Sofa
     ↓ tap
Movie mode

🖥️ Desk
     ↓ tap
Work mode
```

You could literally have:

> Tap phone on desk → lights to 20% + AC 24°C + curtains closed + TV on.

And because you already have your own app/backend, the NFC tag doesn't even need to contain the whole action. It can just be:

```text
zone = bedroom
action = sleep
```

---

### 4. BME680 🌡️

[BME680](https://robocraze.com/products/bme680-temperature-sensor-module)

This gives you **temperature + humidity + pressure + gas/VOC-related air-quality information** in one sensor. Robocraze currently lists it at ₹829. [Robocraze](https://robocraze.com/products/bme680-temperature-sensor-module?utm_source=chatgpt.com)

Now your AC controller becomes an environmental node:

```text
26.8°C
61% RH
1011 hPa
air-quality reading
```

Then you can build actual automation logic:

```text
IF room occupied
AND temperature > 26
AND humidity > 60%
THEN
    AC = ON
    target = 23
```

For even more serious indoor-air monitoring, Robocraze has an SCD40 breakout at ₹2,705 that measures CO₂, temperature and humidity over I2C, although the listing has some inconsistent CO/CO₂ wording, so I'd verify the exact module before buying. [Robocraze](https://robocraze.com/products/scd40-i2c-co-temperature-humidity-sensor-breakout-module-7semi?utm_source=chatgpt.com)

---

### 5. 1.3 inch OLED 🖥️

[1.3 inch I2C OLED](https://robocraze.com/products/1-3in-oled-display)

₹275 currently.

This turns the breadboard monster into something that actually has a UI.

You could display:

```text
┌────────────────────┐
│ BEDROOM             │
│                     │
│ 26.7°C     61%      │
│                     │
│ AC       ON         │
│ TARGET   23°C       │
│                     │
│ OCCUPIED             │
│ WiFi ✓              │
└────────────────────┘
```

It's I2C, so it takes very few GPIOs. [Robocraze](https://robocraze.com/products/1-3in-oled-display?utm_source=chatgpt.com)

I'd definitely get one.

---

### 6. WS2812B addressable RGB

[7Semi WS2812B Digital RGB LED](https://robocraze.com/products/7semi-ws2812b-digital-rgb-led-module)

The individual module is ₹64 and can be controlled one-wire from the ESP32. [Robocraze](https://robocraze.com/products/7semi-ws2812b-digital-rgb-led-module?utm_source=chatgpt.com)

But for a genuinely fun project, eventually buy a **long strip**.

Then:

```text
ESP32
 ↓
WS2812B
 ↓
████████████████████
```

You can make it:

**AC status light**

```text
Blue  = cooling
Orange = warming
Red = too hot
Green = comfortable
```

or:

**Music visualizer**

```text
Microphone
   ↓
FFT
   ↓
LED animation
```

or:

**PC Ambilight**

```text
Mac display
    ↓
screen color sampling
    ↓
Wi-Fi
    ↓
ESP32
    ↓
WS2812B
```

Robocraze also has a 5V 5A supply for ₹335 if you later run a sizeable LED strip. [Robocraze](https://robocraze.com/products/5v-5a-smps-power-supply-1?utm_source=chatgpt.com)

---

### 7. VL53L0X ToF laser distance sensor

[VL53L0X](https://robocraze.com/products/vl53l0x-laser-ranging-sensor)

₹126.

This is much more interesting than HC-SR04 for compact projects. It's a time-of-flight distance sensor, with a listed range around 2 m. [Robocraze](https://robocraze.com/products/vl53l0x-laser-ranging-sensor?utm_source=chatgpt.com)

You can make:

```text
Hand approaches
      ↓
distance = 8 cm
      ↓
gesture recognized
      ↓
AC temperature +1
```

So:

```text
hand close
→ next action

hand far
→ previous action
```

You could literally make a **touchless AC controller**.

---

### 8. MPR121, 12-channel capacitive touch

[MPR121](https://robocraze.com/products/mpr121-12-channel-capacitive-touch-sensor-controller-module-i2c-interface)

₹144.

This is absurdly useful.

It gives you **12 capacitive touch inputs over I2C**. [Robocraze](https://robocraze.com/products/mpr121-12-channel-capacitive-touch-sensor-controller-module-i2c-interface?utm_source=chatgpt.com)

And because they're capacitive, your input doesn't need to be a conventional button.

You can connect:

```text
copper tape
metal objects
door handles
foil
custom PCB traces
```

Then make a physical smart-home panel:

```text
┌─────────────────────────┐
│  LIGHT       AC         │
│                         │
│  TAP         TAP        │
│                         │
│  MOVIE       SLEEP      │
│                         │
│  AWAY        HOME       │
└─────────────────────────┘
```

No mechanical switches.

---

### 9. NRF24L01 + PA/LNA 📡

[NRF24L01+PA+LNA](https://robocraze.com/products/nrf24l01-pa-lna)

₹155.

This is a fun one because now you can build **your own private RF network** between ESP32 devices. Robocraze lists a 2.4 GHz transceiver with PA/LNA and SMA antenna. [Robocraze](https://robocraze.com/products/nrf24l01-pa-lna?utm_source=chatgpt.com)

You could have:

```text
                     Wi-Fi
                       │
                    ESP32 HQ
                       │
                  NRF24 network
             ┌─────────┼─────────┐
             ↓         ↓         ↓
          node #1   node #2   node #3
          curtains   sensors   lights
```

This is useful where you don't want every tiny sensor to join Wi-Fi.

---

### 10. MAX485 / RS485

[7Semi MAX485](https://robocraze.com/products/7semi-rs485-to-uart-converter-breakout-module-max485)

₹138.

This one is **very engineer-y**.

RS485 lets your ESP32 talk to industrial devices over long cable runs and noisy environments. The Robocraze module lists up to 1200 m under ideal conditions. [Robocraze](https://robocraze.com/products/7semi-rs485-to-uart-converter-breakout-module-max485?utm_source=chatgpt.com)

Then you can start interfacing with actual building automation hardware:

```text
ESP32
 ↓
MAX485
 ↓
RS485
 ↓
Modbus device
```

Temperature controllers, energy meters, industrial sensors, VFDs, HVAC controllers, etc.

This is where your weekend project starts becoming actual industrial automation.

---

### 11. INA219 power monitor ⚡

[INA219](https://robocraze.com/products/ina219-i2c-voltage-current-power-monitor-sensor-breakout-board-7semi)

₹240.

It measures current, voltage and power over I2C, with a listed 0 to 26 V range. [Robocraze](https://robocraze.com/products/ina219-i2c-voltage-current-power-monitor-sensor-breakout-board-7semi?utm_source=chatgpt.com)

You can make:

```text
Power supply
    ↓
 INA219
    ↓
 ESP32
    ↓
 dashboard
```

Then show:

```text
ESP32 node

Voltage: 5.04 V
Current: 0.42 A
Power:   2.12 W
Energy:  0.31 Wh
```

Very useful when debugging your other projects.

---

### 12. SG90 servo + PCA9685 🤖

[SG90 servo](https://robocraze.com/products/sg90-micro-servo-motor)  
[PCA9685 16-channel servo driver](https://robocraze.com/products/pca9685-16-channel-12-bit-servo-motor-driver)

The SG90 is around ₹97. The PCA9685 gives you **16 PWM channels over just two I2C pins**, which is fantastic if you start attaching lots of motors. [Robocraze](https://robocraze.com/products/sg90-micro-servo-motor?utm_source=chatgpt.com)

Now your ESP32 can physically move things:

```text
ESP32
  ↓
PCA9685
 ├── servo 1
 ├── servo 2
 ├── servo 3
 ...
 └── servo 16
```

Suddenly:

> Alexa, close the physical curtain.

becomes possible.

Or build a little pan/tilt mount for your IR transmitter.

---

### 13. DFPlayer Mini 🔊

[DFPlayer Mini](https://robocraze.com/products/dfplayer-mini-mp3-module)

₹95.

It reads audio from a microSD card and can directly drive a small speaker. [Robocraze](https://robocraze.com/products/dfplayer-mini-mp3-module?utm_source=chatgpt.com)

Now your automation node can **talk**.

```text
Someone enters room
      ↓
mmWave
      ↓
ESP32
      ↓
DFPlayer
      ↓
Good evening.
AC temperature is 23 degrees.
```

Or:

```text
Door opened
 ↓
Welcome home Shravan
```

Pair this with the microphone and now you have:

```text
ears + brain + voice + Wi-Fi + actuators
```

That's where things start getting really fun.

---

### 14. MPU6050 motion sensor

[MPU6050](https://robocraze.com/products/mpu-6050-triple-axis-accelerometer-gyroscope-module)

₹154.

Accelerometer + gyroscope over I2C. [Robocraze](https://robocraze.com/products/mpu-6050-triple-axis-accelerometer-gyroscope-module?utm_source=chatgpt.com)

You can use it for:

```text
gesture controller
shake detection
tilt controller
robot balancing
orientation tracking
motion logging
```

For example, build a physical remote:

```text
Tilt left  → AC temp down
Tilt right → AC temp up
Shake      → AC off
```

Ridiculous, but completely doable.

---

### 15. GPS

[NEO-6M GPS](https://robocraze.com/products/neo-6m-gps-module)

₹266 currently.

Now your ESP32 becomes a location-aware device. The listing specifies UART and an external antenna. [Robocraze](https://robocraze.com/products/neo-6m-gps-module?utm_source=chatgpt.com)

This is more interesting outdoors:

```text
ESP32
 +
GPS
 +
Wi-Fi/Bluetooth
 +
battery
```

Build:

**GPS tracker**

**geofenced automation**

**bike tracker**

**asset tracker**

**location-aware data logger**

---

# My actual top 7 for YOU

Given what you've already built, I wouldn't buy 20 random sensors.

I'd buy these:

| Priority | Module | Why |
|---|---|---|
| 🔥🔥🔥 | **24 GHz mmWave C4001** | Real human presence |
| 🔥🔥🔥 | **INMP441** | Audio input |
| 🔥🔥🔥 | **MPR121** | 12-channel touch |
| 🔥🔥🔥 | **1.3 OLED** | Local UI |
| 🔥🔥🔥 | **WS2812B** | Physical visual output |
| 🔥🔥 | **PN532 NFC** | Tap-to-trigger automation |
| 🔥🔥 | **BME680** | Environmental sensing |

Those seven turn your ESP32 into something resembling a **tiny autonomous home-control computer**.

---

# The project I'd build with all of them

This is the one I think you'd genuinely enjoy:

## ESP32 Home Agent Node

```text
                         Wi-Fi
                           │
                    ┌──────▼──────┐
                    │ Your Backend│
                    └──────┬──────┘
                           │
                        ESP32
                           │
       ┌──────────┬────────┼────────┬──────────┐
       │          │        │        │          │
       ▼          ▼        ▼        ▼          ▼
      IR       mmWave    Mic      BME680     NFC
       │          │        │        │          │
      AC        human    audio     room       tap
                 │
                 │
       ┌─────────┴─────────┐
       │                   │
       ▼                   ▼
     OLED               WS2812B
     display              LEDs
       │
       ▼
   status/UI

                     ESP32 outputs
                          │
                  ┌───────┼───────┐
                  ▼       ▼       ▼
                 IR     Servo   Speaker
                 AC     curtain   voice
```

Then the logic becomes absurdly powerful:

```text
07:00
↓
Presence detected
↓
Curtains open

08:00
↓
Room temperature 27°C
↓
AC → 24°C

Someone taps NFC tag
↓
Movie mode
↓
Lights 20%
AC 23°C
Curtains closed
TV command via IR

Nobody detected for 30 minutes
↓
AC OFF
Lights OFF
```

And your **existing physical WAVESS remote still works** because the ESP32 can listen to the IR traffic and maintain a best-effort software state.

---

# A particularly crazy upgrade

Your ESP32 itself has a **CAN-compatible TWAI controller**. [Espressif Documentation](https://documentation.espressif.com/esp32-wroom-32_datasheet_en.html?utm_source=chatgpt.com)

That means you can eventually add a CAN transceiver and start experimenting with **CAN bus**.

That opens an entirely different rabbit hole:

```text
ESP32
 ↓
CAN transceiver
 ↓
CAN bus
 ↓
Car
```

You can learn how automotive ECUs communicate, build a CAN logger, make dashboards, decode frames and build bench simulations.

**Do this on a proper isolated/bench CAN setup first, not by randomly connecting the ESP32 to your car.**

---

## One thing I would NOT buy yet

Don't buy a bunch of different ESP32 development boards.

Your WROOM-32 is already enough for almost all of these projects. The official datasheet gives it Wi-Fi/BLE, I2C, SPI, I2S, IR/RMT, touch, ADC/DAC and TWAI. [Espressif Documentation](https://documentation.espressif.com/esp32-wroom-32_datasheet_en.html?utm_source=chatgpt.com)

Where I'd eventually upgrade to an **ESP32-S3 with PSRAM** is when you get into serious camera/vision, larger displays, audio processing or local AI.

For now, **your current board is the perfect hacker board**.

### If I were placing your next Robocraze order tonight

I'd get:

**C4001 mmWave + INMP441 + 1.3 OLED + MPR121 + WS2812B + PN532 + BME680 + VL53L0X + PCA9685 + 2× SG90 + DFPlayer Mini + INA219 + MAX485 + NRF24L01 PA/LNA.**

That's roughly the point where your breadboard stops being an ESP32 experiment and starts looking like a **Frankenstein home automation platform**. 😄