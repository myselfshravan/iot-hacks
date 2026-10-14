#include "web_api.h"

#include <WiFi.h>

#include "../web/control_html.h"

namespace web_api {
namespace {

constexpr size_t kImageBytes = Screen::kWidth * Screen::kHeight * 2;

WebServer* srv = nullptr;
const Deps* deps = nullptr;
uint16_t* image = nullptr;  // receives /frame uploads
size_t imageFill = 0;
uint32_t imageUntil = 0;

uint32_t secsArg(uint32_t fallbackSecs) {
  return (srv->hasArg("secs") ? srv->arg("secs").toInt() : fallbackSecs) * 1000UL;
}

String jsonEscape(const String& s) {
  String out;
  for (char ch : s) {
    if (ch == '"' || ch == '\\') out += '\\';
    if ((uint8_t)ch >= 0x20) out += ch;
  }
  return out;
}

String lightJson();

String stateJson() {
  String j = "{";
  j += "\"mood\":\"" + String(mood::name(deps->pet.mood())) + "\"";
  j += ",\"base\":\"" + String(mood::name(deps->pet.baseMood())) + "\"";
  j += ",\"pet\":\"" + deps->pets.active() + "\"";
  j += ",\"say\":\"" + jsonEscape(deps->pet.saying()) + "\"";
  const Gesture& g = deps->touch.last();
  j += ",\"gesture\":{\"id\":" + String(g.id) + ",\"presses\":" + String(g.presses) +
       ",\"total_ms\":" + String(g.totalMs) + ",\"press_ms\":[";
  for (int i = 0; i < min<int>(g.presses, Gesture::kMaxPresses); i++)
    j += (i ? "," : "") + String(g.pressMs[i]);
  j += "],\"gap_ms\":[";
  for (int i = 1; i < min<int>(g.presses, Gesture::kMaxPresses); i++)
    j += (i > 1 ? "," : "") + String(g.gapMs[i]);
  j += "]}";
  j += ",\"touching\":" + String(deps->touch.held() ? "true" : "false");
  j += ",\"light\":" + lightJson();
  j += ",\"uptime_s\":" + String(millis() / 1000);
  j += ",\"rssi\":" + String(WiFi.RSSI());
  j += ",\"free_heap\":" + String(ESP.getFreeHeap());
  return j + "}";
}

String lightJson() {
  const Strip& st = deps->strip;
  return String("{\"mode\":\"") + Strip::modeName(st.mode()) + "\",\"bri\":" +
         String(st.brightness()) + ",\"syncing\":" + (st.syncing() ? "true" : "false") + "}";
}

void handleLight() {
  if (srv->hasArg("mode")) {
    Strip::Mode m;
    if (!Strip::modeFromName(srv->arg("mode"), &m)) {
      srv->send(400, "text/plain", "unknown mode");
      return;
    }
    deps->strip.setMode(m);
  }
  if (srv->hasArg("bri")) deps->strip.setBrightness(constrain(srv->arg("bri").toInt(), 0, 255));
  srv->send(200, "application/json", lightJson());
}

void sendState() { srv->send(200, "application/json", stateJson()); }

void handleMood() {
  Mood m;
  if (!mood::fromName(srv->arg("m"), &m)) {
    srv->send(400, "text/plain", "unknown mood");
    return;
  }
  deps->pet.setMood(m, secsArg(0));
  sendState();
}

void handleSay() {
  deps->pet.say(srv->arg("text"), secsArg(8));
  sendState();
}

void handleLook() {
  deps->pet.lookAt(srv->arg("x").toFloat(), srv->arg("y").toFloat(), secsArg(3));
  sendState();
}

void handleFrameUpload() {
  HTTPUpload& up = srv->upload();
  if (up.status == UPLOAD_FILE_START) imageFill = 0;
  if (up.status == UPLOAD_FILE_WRITE && imageFill + up.currentSize <= kImageBytes) {
    memcpy((uint8_t*)image + imageFill, up.buf, up.currentSize);
    imageFill += up.currentSize;
  }
}

void handleFrameDone() {
  if (imageFill != kImageBytes) {
    srv->send(400, "text/plain", "expected 40960 bytes of RGB565");
    return;
  }
  imageUntil = millis() + secsArg(10);
  sendState();
}

}  // namespace

void begin(WebServer& server, const Deps& d) {
  srv = &server;
  deps = new Deps(d);
  image = (uint16_t*)malloc(kImageBytes);
  server.on("/", HTTP_GET, [] { srv->send_P(200, "text/html", kControlHtml); });
  server.on("/state", HTTP_GET, sendState);
  server.on("/mood", HTTP_GET, handleMood);
  server.on("/say", HTTP_GET, handleSay);
  server.on("/look", HTTP_GET, handleLook);
  server.on("/hop", HTTP_GET, [] {
    deps->pet.hop();
    sendState();
  });
  server.on("/light", HTTP_GET, handleLight);
  server.on("/frame", HTTP_POST, handleFrameDone, handleFrameUpload);
  server.on("/think", HTTP_GET, [] {
    deps->pet.think(secsArg(4));
    sendState();
  });
  server.on("/pets", HTTP_GET, [] { srv->send(200, "application/json", deps->pets.listJson()); });
  server.on("/pets/use", HTTP_GET, [] {
    if (!deps->pets.use(srv->arg("name"))) {
      srv->send(404, "text/plain", "no such pet");
      return;
    }
    srv->send(200, "application/json", deps->pets.listJson());
  });
  server.on("/pets/delete", HTTP_GET, [] {
    if (!deps->pets.remove(srv->arg("name"))) {
      srv->send(404, "text/plain", "no such pet");
      return;
    }
    srv->send(200, "application/json", deps->pets.listJson());
  });
  server.on(
      "/pets/upload", HTTP_POST,
      [] {
        if (deps->pets.finishUpload()) srv->send(200, "application/json", deps->pets.listJson());
        else srv->send(400, "text/plain", "upload failed (bad name or flash full)");
      },
      [] {
        HTTPUpload& up = srv->upload();
        if (up.status == UPLOAD_FILE_START) deps->pets.beginUpload(srv->arg("name"));
        else if (up.status == UPLOAD_FILE_WRITE) deps->pets.write(up.buf, up.currentSize);
      });
  server.onNotFound([] { srv->send(404, "text/plain", "Not found"); });
  server.begin();
}

const uint16_t* activeImage(uint32_t now) {
  if (!imageUntil) return nullptr;
  if ((int32_t)(now - imageUntil) >= 0) {
    imageUntil = 0;
    return nullptr;
  }
  return image;
}

}  // namespace web_api
