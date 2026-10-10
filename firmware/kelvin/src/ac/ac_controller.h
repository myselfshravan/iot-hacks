// Owns the AC's state and every path that changes it: app/Alexa/serial
// commands (sent over IR with echo check + retry) and the physical remote
// (decoded from the receiver). Persists the state across reboots.
#pragma once

#include <Arduino.h>
#include <Preferences.h>

#include "../ir/ir_port.h"
#include "../sensors/climate.h"
#include "../ui/status_led.h"
#include "wavess.h"

class AcController {
 public:
  using ChangeListener = void (*)(const wavess::State&);

  AcController(IrPort& ir, Climate& climate, StatusLed& led);

  void begin();  // restores the last state from flash
  void onChange(ChangeListener fn) { listener_ = fn; }

  // Sends `next` to the AC. `source` is "app", "alexa", "serial"...
  void command(const wavess::State& next, const char* source);

  // Feed every received IR frame; adopts it if it's the physical remote.
  void onIrFrame(const decode_results& frame);

  const wavess::State& state() const { return state_; }
  String json() const;

  // Parses one "key=value" field (power, mode, temp, fan, swing) into `s`.
  static bool applyField(wavess::State& s, const String& key, const String& value);

 private:
  void adopt(const wavess::State& next, const char* source);
  bool waitForEcho(uint64_t bits);
  int expectedAirMove(const wavess::State& before) const;

  IrPort& ir_;
  Climate& climate_;
  StatusLed& led_;
  Preferences prefs_;
  ChangeListener listener_ = nullptr;

  wavess::State state_;
  const char* source_ = "boot";
  uint32_t seq_ = 0;  // bumps on every change
  int echo_ = -1;     // -1 n/a, 0 not heard, 1 heard
};
