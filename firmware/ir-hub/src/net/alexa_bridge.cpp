#include "alexa_bridge.h"

#include <Espalexa.h>

namespace alexa {
namespace {

Espalexa espalexa;
EspalexaDevice* device = nullptr;
AcController* acRef = nullptr;

void onAlexa(EspalexaDevice* d) {
  if (!d || !acRef) return;
  wavess::State next = acRef->state();
  next.power = d->getValue() > 0;
  uint8_t pct = d->getPercent();
  if (next.power && pct >= wavess::kMinTemp && pct <= wavess::kMaxTemp) next.temp = pct;
  acRef->command(next, "alexa");
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

void loop() { espalexa.loop(); }

bool handle(const String& uri, const String& body) {
  return espalexa.handleAlexaApiCall(uri, body);
}

void reflect(const wavess::State& s) {
  if (!device) return;
  device->setState(s.power);
  device->setPercent(s.temp);
}

}  // namespace alexa
