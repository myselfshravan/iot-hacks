#include "web_api.h"

#include <WiFi.h>

#include "../remotes/projector.h"
#include "../web/index_html.h"
#include "alexa_bridge.h"

namespace web_api {
namespace {

WebServer* srv = nullptr;
const Deps* deps = nullptr;

void handleSet() {
  wavess::State next = deps->ac.state();
  for (int i = 0; i < srv->args(); i++) {
    if (!AcController::applyField(next, srv->argName(i), srv->arg(i))) {
      srv->send(400, "text/plain", "bad " + srv->argName(i));
      return;
    }
  }
  deps->ac.command(next, "app");
  srv->send(200, "application/json", deps->ac.json());
}

void handleProjector() {
  const projector::Button* b = projector::find(srv->arg("b"));
  if (!b) {
    srv->send(400, "text/plain", "unknown button");
    return;
  }
  deps->led.flashTx();
  projector::send(deps->ir.tx(), *b);
  Serial.printf("TX projector %s\n", b->id);
  srv->send(200, "text/plain", "ok");
}

}  // namespace

String healthJson(const Deps& d) {
  String j = "{";
  j += "\"uptime_s\":" + String(millis() / 1000);
  j += ",\"wifi\":" + String(d.wifi.online() ? "true" : "false");
  j += ",\"ip\":\"" + WiFi.localIP().toString() + "\"";
  j += ",\"rssi\":" + String(WiFi.RSSI());
  j += ",\"reconnects\":" + String(d.wifi.reconnects());
  j += ",\"free_heap\":" + String(ESP.getFreeHeap());
  j += ",\"reset_reason\":" + String((int)esp_reset_reason());
  j += ",\"sensor\":" + String(d.climate.ok() ? "true" : "false");
  return j + "}";
}

void registerRoutes(WebServer& server, const Deps& d) {
  srv = &server;
  deps = new Deps(d);
  server.on("/", HTTP_GET, [] { srv->send_P(200, "text/html", kIndexHtml); });
  server.on("/state", HTTP_GET, [] { srv->send(200, "application/json", deps->ac.json()); });
  server.on("/health", HTTP_GET,
            [] { srv->send(200, "application/json", healthJson(*deps)); });
  server.on("/set", HTTP_GET, handleSet);
  server.on("/projector", HTTP_GET, handleProjector);
  server.onNotFound([] {
    if (!alexa::handle(srv->uri(), srv->arg(0))) srv->send(404, "text/plain", "Not found");
  });
}

}  // namespace web_api
