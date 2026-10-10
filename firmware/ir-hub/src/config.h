// Pins and tunables for the IR hub (ESP32 #1). Wiring: docs/wiring.md.
#pragma once

#include <Arduino.h>

namespace config {

constexpr char kHostname[] = "irhub";       // http://irhub.local
constexpr char kAlexaName[] = "Room AC";    // "AC" is taken by a Control4 scene

// Pins
constexpr uint8_t kIrRecvPin = 14;      // VS1838B OUT
constexpr uint8_t kIrRecvPowerPin = 27; // VS1838B VCC (driven HIGH)
constexpr uint8_t kIrSendPin = 26;      // -> 220R -> 2N2222A base -> IR LED
constexpr uint8_t kStatusLedPin = 2;    // onboard blue LED
constexpr uint8_t kDhtPin = 25;
constexpr uint8_t kDhtPowerPin = 33;    // DHT VCC (driven HIGH)

// AC delivery
constexpr int kMaxSendAttempts = 3;     // resend until our receiver hears the echo
constexpr uint32_t kEchoWaitMs = 250;   // a WAVESS frame lasts ~90 ms

// Robustness
constexpr uint32_t kWatchdogMs = 30000; // reboot if loop() stalls this long

}  // namespace config
