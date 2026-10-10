// Pins and tunables for Dexi (ESP32 #2). Wiring: docs/wiring.md (Dexi).
#pragma once

#include <Arduino.h>

namespace config {

constexpr char kHostname[] = "dexi";  // http://dexi.local

// 1.8" ST7735R TFT (hardware SPI: SCK 18, MOSI 23)
constexpr uint8_t kTftCs = 5;
constexpr uint8_t kTftDc = 16;
constexpr uint8_t kTftRst = 17;
constexpr uint8_t kTftBacklight = 4;
constexpr uint8_t kTftRotation = 2;      // header at the top as it sits on the desk
constexpr uint32_t kTftSpiHz = 20000000;

// TTP223 touch pad. The breadboard has no spare 3V3/GND holes next to the
// board, so the pad is powered from two GPIOs (it draws only microamps).
constexpr uint8_t kTouchPin = 19;       // TTP223 I/O (HIGH while touched)
constexpr uint8_t kTouchPowerPin = 21;  // TTP223 VCC (driven HIGH)
constexpr uint8_t kTouchGroundPin = 22; // TTP223 GND (driven LOW)
constexpr uint32_t kLongPressMs = 1200;

constexpr uint32_t kFrameMs = 33;       // ~30 fps
constexpr uint32_t kWatchdogMs = 30000;

}  // namespace config
