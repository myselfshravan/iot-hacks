// IR hub (ESP32 #1): replicates IR remotes over Wi-Fi.
//   Kelvin    the WAVESS AC remote (main job): http://irhub.local, Alexa "Room AC"
//   Projector side extra on the same IR LED:   http://irhub.local/projector
//
// Kelvin builds WAVESS frames from a state model, checks that the hub's own
// receiver heard each one (resending if not), follows the physical remote, and
// uses a DHT in the outlet air to confirm the AC really reacted.
//
// Code map (src/; Wi-Fi comes from the shared libraries/WifiLink):
//   config.h               pins and tunables
//   ac/wavess.*            WAVESS protocol (captures/wavess-protocol.md)
//   ac/ac_controller.*     AC state, send + echo/retry, remote tracking, persistence
//   ir/ir_port.*           IR LED + receiver hardware
//   projector/projector.*  projector NEC codes
//   sensors/climate.*      DHT readings + reaction confirmation
//   ui/status_led.h        blue LED: solid online, slow blink offline, flicker sending
//   net/web_api.*          HTTP routes
//   net/alexa_bridge.*     Alexa (Hue emulation)
//   web/*.html             phone UI + projector page (embedded as *_html.h by tools/embed_web.py)
//   serial_console.*       USB serial commands
//
// Wi-Fi credentials: secrets.h (copy secrets.example.h). Wiring: docs/wiring.md.

#include <ESPmDNS.h>
#include <WiFi.h>
#include <esp_task_wdt.h>

#include "secrets.h"
#include "src/ac/ac_controller.h"
#include "src/config.h"
#include "src/ir/ir_port.h"
#include "src/net/alexa_bridge.h"
#include "src/net/web_api.h"
#include <WifiLink.h>
#include "src/sensors/climate.h"
#include "src/serial_console.h"
#include "src/ui/status_led.h"

IrPort ir(config::kIrSendPin, config::kIrRecvPin, config::kIrRecvPowerPin);
Climate climate(config::kDhtPin, config::kDhtPowerPin);
StatusLed led(config::kStatusLedPin);
AcController ac(ir, climate, led);
WifiLink wifi;
WebServer server(80);
web_api::Deps deps{ac, ir, climate, led, wifi};
bool servicesUp = false;

// secrets.h lists one or more networks; Kelvin joins whichever is in range.
#ifdef WIFI_NETWORKS
const WifiLink::Network kNetworks[] = WIFI_NETWORKS;
#else
const WifiLink::Network kNetworks[] = {{WIFI_SSID, WIFI_PASSWORD}};
#endif

void startServices() {
  if (MDNS.begin(config::kHostname)) MDNS.addService("http", "tcp", 80);
  web_api::registerRoutes(server, deps);
  alexa::begin(server, ac, config::kAlexaName);
  servicesUp = true;
  Serial.printf("READY  http://%s.local  or  http://%s\n", config::kHostname,
                WiFi.localIP().toString().c_str());
}

void startWatchdog() {
  esp_task_wdt_config_t cfg = {
      .timeout_ms = config::kWatchdogMs, .idle_core_mask = 0, .trigger_panic = true};
  if (esp_task_wdt_reconfigure(&cfg) != ESP_OK) esp_task_wdt_init(&cfg);
  esp_task_wdt_add(nullptr);
}

void setup() {
  Serial.begin(115200);
  Serial.printf("\nIR hub boot (reset reason %d)\n", (int)esp_reset_reason());
  led.begin();
  ir.begin();
  climate.begin();
  ac.begin();
  ac.onChange(alexa::reflect);
  serial_console::begin(ac, [] { return web_api::healthJson(deps); });
  wifi.begin(config::kHostname, kNetworks, sizeof(kNetworks) / sizeof(kNetworks[0]));
  wifi.onFirstConnect(startServices);
  startWatchdog();
}

void loop() {
  esp_task_wdt_reset();
  wifi.update();
  if (servicesUp) alexa::loop();
  serial_console::update();
  if (ir.receive()) ac.onIrFrame(ir.frame());
  climate.update();
  led.update(wifi.online());
}
