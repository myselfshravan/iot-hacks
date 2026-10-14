// Pins and tunables for Dexi (ESP32 #2). Wiring: docs/wiring.md (Dexi).
#pragma once

#include <Arduino.h>

namespace config {

constexpr char kHostname[] = "dexi";  // http://dexi.local
constexpr char kTimezone[] = "IST-5:30";  // POSIX TZ for the sky and clock

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

// WS2812B strip behind the monitor (own 5 V 2 A adapter, common GND).
constexpr uint8_t kLedPin = 25;          // DIN through 220R
constexpr uint16_t kLedCount = 103;      // from bottom-right, anticlockwise: right 20, top 32, left 19, bottom 32
constexpr uint16_t kLedMaxMa = 1800;     // stays under the 2 A adapter
constexpr uint16_t kLedUdpPort = 5568;   // HyperHDR "udpraw" device

constexpr uint32_t kFrameMs = 33;       // ~30 fps
constexpr uint32_t kWatchdogMs = 30000;

}  // namespace config
