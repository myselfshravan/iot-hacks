// Exposes the AC to Alexa as a dimmable light (Hue bridge emulation, local LAN,
// no skill or cloud). On/off = power, brightness % = temperature °C.
#pragma once

#include <WebServer.h>

#include "../ac/ac_controller.h"

namespace alexa {

void begin(WebServer& server, AcController& ac, const char* name);
void loop();  // also services the web server

// For WebServer::onNotFound: true if it was an Alexa API call.
bool handle(const String& uri, const String& body);

// Mirrors the AC state into what Alexa reports.
void reflect(const wavess::State& s);

}  // namespace alexa
