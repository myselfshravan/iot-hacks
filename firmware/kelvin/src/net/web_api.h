// HTTP API + phone UI.
//   GET /                -> phone UI (src/web/index.html)
//   GET /state           -> AC state + delivery feedback (JSON)
//   GET /set?power=1&mode=cool&temp=24&fan=auto&swing=1   (any subset)
//   GET /projector?b=ok  -> one projector button
//   GET /health          -> uptime, Wi-Fi, heap, sensor (JSON)
#pragma once

#include <WebServer.h>

#include "../ac/ac_controller.h"
#include "../ir/ir_port.h"
#include "../sensors/climate.h"
#include "../ui/status_led.h"
#include "wifi_link.h"

namespace web_api {

struct Deps {
  AcController& ac;
  IrPort& ir;
  Climate& climate;
  StatusLed& led;
  WifiLink& wifi;
};

// Registers routes; the server itself is started by alexa::begin().
void registerRoutes(WebServer& server, const Deps& deps);

String healthJson(const Deps& deps);

}  // namespace web_api
