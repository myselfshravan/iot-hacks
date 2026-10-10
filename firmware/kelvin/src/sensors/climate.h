// DHT air sensor + "did the AC actually react?" confirmation.
//
// Place the sensor in the FCU's outlet airflow. After a power/mode change the
// outlet air moves several degrees within ~1-2 min: cooling on -> drops,
// AC off -> rises back toward room temperature. If the expected move happens
// within kWindowMs the command is "confirmed", otherwise "unconfirmed".
// With the sensor elsewhere (or absent) it just reports room readings.
#pragma once

#include <Arduino.h>
#include <DHT.h>

class Climate {
 public:
  enum class Confirm { kNone, kPending, kConfirmed, kUnconfirmed };

  // Flip to DHT22 if the module is the white one (DHT11 is blue).
  static constexpr uint8_t kType = DHT11;
  static constexpr float kReactionC = 1.5;       // movement that counts as a reaction
  static constexpr uint32_t kWindowMs = 180000;  // how long to wait for it
  static constexpr uint32_t kReadEveryMs = 5000;

  Climate(uint8_t dataPin, uint8_t powerPin);

  void begin();
  void update();  // call every loop; reads every kReadEveryMs

  // direction: -1 air should get colder, +1 warmer, 0 nothing to confirm.
  void expect(int direction);

  bool ok() const { return !isnan(tempC_); }
  float tempC() const { return tempC_; }
  float humidity() const { return humidity_; }
  Confirm confirm() const { return confirm_; }
  static const char* name(Confirm c);

 private:
  uint8_t powerPin_;
  DHT dht_;
  float tempC_ = NAN, humidity_ = NAN, baselineC_ = NAN;
  int direction_ = 0;
  uint8_t badReads_ = 0;
  uint32_t lastReadMs_ = 0, expectSinceMs_ = 0;
  Confirm confirm_ = Confirm::kNone;
};
