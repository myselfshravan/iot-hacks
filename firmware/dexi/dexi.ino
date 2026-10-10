// Dexi: an AI desk pet on a 1.8" TFT (ESP32 #2).
//
// Dexi is a chibi pixel creature (Clawd-style) living in a little world whose sky follows the real
// time of day. The ESP32 keeps it alive on its own: it wanders, hops, blinks,
// sleeps and reacts to being petted (TTP223 touch pad), with or without Wi-Fi. The brain (services/dexi-brain on the laptop) decides moods and lines
// via an LLM and drives this board over HTTP; it can also push full-screen
// images rendered on the laptop.
//
//   Phone:  http://dexi.local       API: src/net/web_api.h
//
// Code map (src/; Wi-Fi comes from the shared libraries/WifiLink):
//   config.h            pins and tunables
//   display/screen.*    TFT + off-screen canvas (flicker-free)
//   pet/mood.*          moods: eyes, mouth, colour, behaviour of each
//   pet/sprites.h       the pixel art (16x12 chibi blob at 4x)
//   pet/pet.*           behaviour + animation: wander, hop, sleep, effects, bubble
//   pet/world.*         sky by real time of day (NTP), sun/moon, stars, grass, clock
//   input/touch.*       TTP223 tap / double tap / long press
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
#include "src/pet/pet.h"
#include "src/pet/world.h"
#include "src/input/touch.h"
#include "src/net/web_api.h"

#ifdef WIFI_NETWORKS
const WifiLink::Network kNetworks[] = WIFI_NETWORKS;
#else
const WifiLink::Network kNetworks[] = {{WIFI_SSID, WIFI_PASSWORD}};
#endif

Screen screen(config::kTftCs, config::kTftDc, config::kTftRst, config::kTftBacklight);
Pet pet;
Touch touch(config::kTouchPin, config::kTouchPowerPin, config::kTouchGroundPin,
            config::kLongPressMs);
WifiLink wifi;
WebServer server(80);
bool servicesUp = false;

void startServices() {
  if (MDNS.begin(config::kHostname)) MDNS.addService("http", "tcp", 80);
  web_api::begin(server, {pet, touch, wifi});
  servicesUp = true;
  Serial.printf("READY  http://%s.local  or  http://%s\n", config::kHostname,
                WiFi.localIP().toString().c_str());
  world::startClock(config::kTimezone);
  pet.setMood(Mood::kHappy, 2500);
  pet.hop();
  pet.say("I'm online!", 3000);
}

// Instant local reactions; the brain also sees the counters in /state and
// follows up (tap: a thought, double tap: an info card, long press: a cuddle).
void onTouch(Touch::Event e) {
  switch (e) {
    case Touch::Event::kTap:
      pet.hop();
      pet.setMood(Mood::kHappy, 2500);
      Serial.println("touch: tap");
      break;
    case Touch::Event::kDoubleTap:
      pet.hop();
      pet.setMood(Mood::kSurprised, 1500);
      Serial.println("touch: double tap");
      break;
    case Touch::Event::kLongPress:
      pet.setMood(Mood::kLove, 5000);
      pet.say("purrrr...", 3000);
      Serial.println("touch: long press");
      break;
    default:
      break;
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
  pet.setMood(Mood::kSleepy, 2000);  // waking up
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

  pet.update(now);
  if (const uint16_t* img = web_api::activeImage(now)) {
    screen.present(img);
  } else {
    pet.draw(screen.canvas(), now);
    screen.present();
  }
}
