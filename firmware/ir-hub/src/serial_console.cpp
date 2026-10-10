#include "serial_console.h"

namespace serial_console {
namespace {

AcController* acRef = nullptr;
HealthFn healthFn = nullptr;

void handleSet(String args) {
  wavess::State next = acRef->state();
  while (args.length()) {
    int sp = args.indexOf(' ');
    String tok = sp < 0 ? args : args.substring(0, sp);
    args = sp < 0 ? "" : args.substring(sp + 1);
    int eq = tok.indexOf('=');
    if (eq < 0 || !AcController::applyField(next, tok.substring(0, eq), tok.substring(eq + 1))) {
      Serial.println("ERR bad field: " + tok);
      return;
    }
  }
  acRef->command(next, "serial");
}

}  // namespace

void begin(AcController& ac, HealthFn health) {
  acRef = &ac;
  healthFn = health;
  Serial.println("Serial: set mode=cool temp=24 fan=low swing=1 power=1 | state | health");
}

void update() {
  if (!Serial.available()) return;
  String line = Serial.readStringUntil('\n');
  line.trim();
  if (line == "state") Serial.println(acRef->json());
  else if (line == "health") Serial.println(healthFn());
  else if (line.startsWith("set ")) handleSet(line.substring(4));
  else Serial.println("ERR: set k=v [k=v...] | state | health");
}

}  // namespace serial_console
