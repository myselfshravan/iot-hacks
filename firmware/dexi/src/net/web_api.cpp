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

String stateJson() {
  String j = "{";
  j += "\"mood\":\"" + String(mood::name(deps->face.mood())) + "\"";
  j += ",\"base\":\"" + String(mood::name(deps->face.baseMood())) + "\"";
  j += ",\"say\":\"" + jsonEscape(deps->face.saying()) + "\"";
  j += ",\"taps\":" + String(deps->touch.taps());
  j += ",\"long_presses\":" + String(deps->touch.longPresses());
  j += ",\"uptime_s\":" + String(millis() / 1000);
  j += ",\"rssi\":" + String(WiFi.RSSI());
  j += ",\"free_heap\":" + String(ESP.getFreeHeap());
  return j + "}";
}

void sendState() { srv->send(200, "application/json", stateJson()); }

void handleMood() {
  Mood m;
  if (!mood::fromName(srv->arg("m"), &m)) {
    srv->send(400, "text/plain", "unknown mood");
    return;
  }
  deps->face.setMood(m, secsArg(0));
  sendState();
}

void handleSay() {
  deps->face.say(srv->arg("text"), secsArg(8));
  sendState();
}

void handleLook() {
  deps->face.lookAt(srv->arg("x").toFloat(), srv->arg("y").toFloat(), secsArg(3));
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
  server.on("/frame", HTTP_POST, handleFrameDone, handleFrameUpload);
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
