// Dexi: an AI desk pet on a 1.8" TFT (ESP32 #2).
//
// The ESP32 keeps Dexi alive on its own: it blinks, glances, breathes, morphs
// between moods and reacts to being petted (TTP223 touch pad), with or without
// Wi-Fi. The brain (services/dexi-brain on the laptop) decides moods and lines
// via an LLM and drives this board over HTTP; it can also push full-screen
// images rendered on the laptop.
//
//   Phone:  http://dexi.local       API: src/net/web_api.h
//
// Code map (src/; Wi-Fi comes from the shared libraries/WifiLink):
//   config.h            pins and tunables
//   display/screen.*    TFT + off-screen canvas (flicker-free)
//   face/mood.*         moods and the geometry/colour of each
//   face/face.*         animated face: morphing, blinks, glances, extras, speech bubble
//   input/touch.*       TTP223 taps and long presses
//   net/web_api.*       HTTP API + pushed images
//   web/control.html    phone control page (embedded by tools/embed_web.py)
//
// Wi-Fi credentials: secrets.h (copy secrets.example.h). Wiring: docs/wiring.md.

#include <ESPmDNS.h>
#include <WebServer.h>
#include <WiFi.h>
#include <WifiLink.h>
#include <esp_task_wdt.h>

#include "secrets.h"
#include "src/config.h"
#include "src/display/screen.h"
#include "src/face/face.h"
#include "src/input/touch.h"
#include "src/net/web_api.h"

#ifdef WIFI_NETWORKS
const WifiLink::Network kNetworks[] = WIFI_NETWORKS;
#else
const WifiLink::Network kNetworks[] = {{WIFI_SSID, WIFI_PASSWORD}};
#endif

Screen screen(config::kTftCs, config::kTftDc, config::kTftRst, config::kTftBacklight);
Face face;
Touch touch(config::kTouchPin, config::kTouchPowerPin, config::kTouchGroundPin,
            config::kLongPressMs);
WifiLink wifi;
WebServer server(80);
bool servicesUp = false;

void startServices() {
  if (MDNS.begin(config::kHostname)) MDNS.addService("http", "tcp", 80);
  web_api::begin(server, {face, touch, wifi});
  servicesUp = true;
  Serial.printf("READY  http://%s.local  or  http://%s\n", config::kHostname,
                WiFi.localIP().toString().c_str());
  face.setMood(Mood::kHappy, 2500);
  face.say("I'm online!", 3000);
}

void onTouch(Touch::Event e) {
  if (e == Touch::Event::kTap) {
    face.setMood(Mood::kLove, 3000);
    Serial.println("touch: tap");
  } else if (e == Touch::Event::kLongPress) {
    face.setMood(Mood::kHappy, 5000);
    face.say("purrrr...", 3000);
    Serial.println("touch: long press");
  }
}

void startWatchdog() {
  esp_task_wdt_config_t cfg = {
      .timeout_ms = config::kWatchdogMs, .idle_core_mask = 0, .trigger_panic = true};
  if (esp_task_wdt_reconfigure(&cfg) != ESP_OK) esp_task_wdt_init(&cfg);
  esp_task_wdt_add(nullptr);
}

void setup() {
  Serial.begin(115200);
  Serial.printf("\nDexi boot (reset reason %d)\n", (int)esp_reset_reason());
  randomSeed(esp_random());
  screen.begin(config::kTftRotation, config::kTftSpiHz);
  touch.begin();
  face.setMood(Mood::kSleepy, 2000);  // waking up
  wifi.begin(config::kHostname, kNetworks, sizeof(kNetworks) / sizeof(kNetworks[0]));
  wifi.onFirstConnect(startServices);
  startWatchdog();
}

void loop() {
  static uint32_t lastFrame = 0;
  esp_task_wdt_reset();
  wifi.update();
  if (servicesUp) server.handleClient();

  uint32_t now = millis();
  onTouch(touch.update(now));
  if (now - lastFrame < config::kFrameMs) return;
  lastFrame = now;

  face.update(now);
  if (const uint16_t* img = web_api::activeImage(now)) {
    screen.present(img);
  } else {
    face.draw(screen.canvas(), now);
    screen.present();
  }
}
