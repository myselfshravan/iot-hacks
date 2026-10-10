// USB serial commands (115200 baud), handy when Wi-Fi is down:
//   set mode=cool temp=24 fan=low swing=1 power=1
//   state
//   health
#pragma once

#include <Arduino.h>

#include "ac/ac_controller.h"

namespace serial_console {

using HealthFn = String (*)();

void begin(AcController& ac, HealthFn health);
void update();  // call every loop

}  // namespace serial_console
