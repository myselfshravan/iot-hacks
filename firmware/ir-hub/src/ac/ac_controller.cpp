#include "ac_controller.h"

#include <WiFi.h>

#include "../config.h"

namespace {

bool parseBool(const String& v) { return v == "1" || v == "on" || v == "true"; }

int indexOf(const char* const* list, int n, const String& v) {
  for (int i = 0; i < n; i++)
    if (v.equalsIgnoreCase(list[i])) return i;
  return -1;
}

bool isCooling(const wavess::State& s) {
  return s.power && (s.mode == wavess::kModeCool || s.mode == wavess::kModeDry);
}

String jsonNumber(float v) { return isnan(v) ? "null" : String(v, 1); }

}  // namespace

AcController::AcController(IrPort& ir, Climate& climate, StatusLed& led)
    : ir_(ir), climate_(climate), led_(led) {}

void AcController::begin() {
  prefs_.begin("kelvin");
  if (prefs_.getBytesLength("ac") == sizeof(state_))
    prefs_.getBytes("ac", &state_, sizeof(state_));
}

void AcController::command(const wavess::State& next, const char* source) {
  uint64_t bits = wavess::encode(next);
  led_.flashTx();
  echo_ = 0;
  for (int attempt = 1; attempt <= config::kMaxSendAttempts && !echo_; attempt++) {
    wavess::send(ir_.tx(), bits);
    echo_ = waitForEcho(bits);
    Serial.printf("TX #%d 0x%010llX echo=%s\n", attempt, bits, echo_ ? "yes" : "no");
  }
  adopt(next, source);
}

void AcController::onIrFrame(const decode_results& frame) {
  uint64_t bits;
  wavess::State seen = state_;
  if (!wavess::fromRaw(frame, &bits)) return;
  // Log every WAVESS-shaped frame, so unknown buttons (timer, sleep...) can be
  // captured and decoded from the serial log.
  bool known = wavess::decode(bits, &seen);
  Serial.printf("RX 0x%010llX%s\n", bits, known ? "" : " (unknown layout)");
  if (!known || seen == state_) return;  // unknown, our own echo, or a repeat
  echo_ = -1;
  adopt(seen, "remote");
}

void AcController::adopt(const wavess::State& next, const char* source) {
  wavess::State before = state_;
  state_ = next;
  source_ = source;
  seq_++;
  climate_.expect(expectedAirMove(before));
  prefs_.putBytes("ac", &state_, sizeof(state_));
  if (listener_) listener_(state_);
  Serial.println(json());
}

// Our own receiver sits next to the LED, so it hears every frame we send.
bool AcController::waitForEcho(uint64_t bits) {
  uint32_t start = millis();
  while (millis() - start < config::kEchoWaitMs) {
    uint64_t got;
    if (ir_.receive()) {
      const decode_results& f = ir_.frame();
      if (wavess::fromRaw(f, &got) && got == bits) return true;
      Serial.printf("  echo mismatch: rawlen=%u hdr=%u/%u us\n", f.rawlen,
                    f.rawbuf[1] * kRawTick, f.rawbuf[2] * kRawTick);
    }
    delay(5);
  }
  return false;
}

// Which way the outlet air should move: -1 colder, +1 warmer, 0 no clear signal.
int AcController::expectedAirMove(const wavess::State& before) const {
  if (!isCooling(before) && isCooling(state_)) return -1;
  if (isCooling(before) && !isCooling(state_)) return +1;
  return 0;
}

String AcController::json() const {
  using namespace wavess;
  String j = "{";
  j += "\"power\":" + String(state_.power ? "true" : "false");
  j += ",\"mode\":\"" + String(kModes[state_.mode]) + "\"";
  j += ",\"temp\":" + String(state_.temp);
  j += ",\"fan\":\"" + String(kFans[state_.fan]) + "\"";
  j += ",\"swing\":" + String(state_.swing ? "true" : "false");
  j += ",\"timer\":\"" + String(kTimers[state_.timer]) + "\"";
  j += ",\"hours\":" + String(state_.timerHours);
  j += ",\"sleep\":" + String(state_.sleep ? "true" : "false");
  j += ",\"source\":\"" + String(source_) + "\"";
  j += ",\"seq\":" + String(seq_);
  j += ",\"echo_ok\":" + String(echo_ < 0 ? "null" : echo_ ? "true" : "false");
  j += ",\"confirm\":\"" + String(Climate::name(climate_.confirm())) + "\"";
  j += ",\"air_c\":" + jsonNumber(climate_.tempC());
  j += ",\"air_rh\":" + jsonNumber(climate_.humidity());
  j += ",\"rssi\":" + String(WiFi.RSSI());
  return j + "}";
}

bool AcController::applyField(wavess::State& s, const String& key, const String& value) {
  using namespace wavess;
  if (key == "power") {
    s.power = parseBool(value);
  } else if (key == "swing") {
    s.swing = parseBool(value);
  } else if (key == "timer") {
    int i = indexOf(kTimers, kTimerCount, value);
    if (i < 0) return false;
    s.timer = i;
    if (s.timer != kTimerNone) s.sleep = false;
  } else if (key == "sleep") {
    s.sleep = parseBool(value);
    if (s.sleep) s.timer = kTimerNone;
  } else if (key == "hours") {
    int h = value.toInt();
    if (h < kMinTimerHours || h > kMaxTimerHours) return false;
    s.timerHours = h;
  } else if (key == "temp") {
    int t = value.toInt();
    if (t < kMinTemp || t > kMaxTemp) return false;
    s.temp = t;
  } else if (key == "mode") {
    int i = indexOf(kModes, kModeCount, value);
    if (i < 0) return false;
    s.mode = i;
  } else if (key == "fan") {
    int i = indexOf(kFans, kFanCount, value);
    if (i < 0) return false;
    s.fan = i;
  } else {
    return false;
  }
  return true;
}
