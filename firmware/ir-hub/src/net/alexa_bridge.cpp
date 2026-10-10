#include "alexa_bridge.h"

#include <Espalexa.h>

namespace alexa {
namespace {

Espalexa espalexa;
EspalexaDevice* device = nullptr;
AcController* acRef = nullptr;
wavess::State pendingState;
bool pending = false;
bool sendingForAlexa = false;

// Espalexa calls this mid-request, and Alexa polls the device state right
// after. Sending IR here (up to ~1 s with echo retries) would starve that
// poll and Alexa reports "device is malfunctioning", so only queue the
// command; loop() sends it once the HTTP exchange is done.
void onAlexa(EspalexaDevice* d) {
  if (!d || !acRef) return;
  wavess::State next = pending ? pendingState : acRef->state();
  next.power = d->getValue() > 0;
  uint8_t pct = d->getPercent();
  if (next.power && pct >= wavess::kMinTemp && pct <= wavess::kMaxTemp) next.temp = pct;
  pendingState = next;
  pending = true;
}

}  // namespace

void begin(WebServer& server, AcController& ac, const char* name) {
  acRef = &ac;
  device = new EspalexaDevice(name, onAlexa, EspalexaDeviceType::dimmable,
                              ac.state().power ? 255 : 0);
  device->setPercent(ac.state().temp);
  espalexa.addDevice(device);
  espalexa.begin(&server);  // also starts the web server
}

void loop() {
  espalexa.loop();
  if (pending) {
    pending = false;
    sendingForAlexa = true;
    acRef->command(pendingState, "alexa");
    sendingForAlexa = false;
  }
}

bool handle(const String& uri, const String& body) {
  return espalexa.handleAlexaApiCall(uri, body);
}

void reflect(const wavess::State& s) {
  // After an Alexa command, Alexa reads the light back and expects exactly the
  // on/brightness it just set; rewriting it here makes Alexa report the device
  // as malfunctioning. Only mirror changes from the app, serial or remote.
  if (!device || sendingForAlexa) return;
  device->setState(s.power);
  device->setPercent(s.temp);
}

}  // namespace alexa
