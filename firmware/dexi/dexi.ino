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
//   pet/pet_pack.*      plays a GIF-derived pet pack (.pak) per mood
//   pet/pet_store.*     packs on LittleFS + which pet is active
//   pet/world.*         sky by real time of day (NTP), sun/moon, stars, grass, clock
//   input/touch.*       TTP223 gesture recorder (press durations + gaps)
//   net/web_api.*       HTTP API + pushed images
//   web/control.html    phone control page (embedded by tools/embed_web.py)
//
// Wi-Fi credentials: secrets.h (copy secrets.example.h). Wiring: docs/wiring.md.

#include <ArduinoOTA.h>
#include <ESPmDNS.h>
#include <WebServer.h>
#include <WiFi.h>
#include <WifiLink.h>
#include <esp_task_wdt.h>

#include "secrets.h"
#include "src/config.h"
#include "src/display/screen.h"
#include "src/pet/pet.h"
#include "src/pet/pet_store.h"
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
PetStore pets(pet);
Touch touch(config::kTouchPin, config::kTouchPowerPin, config::kTouchGroundPin);
WifiLink wifi;
WebServer server(80);
bool servicesUp = false;
bool updating = false;  // OTA in progress: stop animating, it starves the upload

void startServices() {
  if (MDNS.begin(config::kHostname)) MDNS.addService("http", "tcp", 80);
  web_api::begin(server, {pet, pets, touch, wifi});
  ArduinoOTA.setHostname(config::kHostname);
  ArduinoOTA.onStart([] {
    updating = true;
    screen.canvas().fillScreen(ST77XX_BLACK);
    screen.canvas().setCursor(28, 70);
    screen.canvas().print("updating...");
    screen.present();
  });
  ArduinoOTA.onProgress([](unsigned done, unsigned total) {
    esp_task_wdt_reset();
    static unsigned lastPct = 101;
    unsigned pct = done * 100 / total;
    if (pct / 10 == lastPct / 10) return;
    lastPct = pct;
    screen.canvas().fillRect(14, 86, pct, 6, ST77XX_GREEN);
    screen.present();
  });
  ArduinoOTA.onError([](ota_error_t) { updating = false; });
  ArduinoOTA.begin();  // `make ota APP=dexi` flashes over Wi-Fi
  servicesUp = true;
  Serial.printf("READY  http://%s.local  or  http://%s\n", config::kHostname,
                WiFi.localIP().toString().c_str());
  world::startClock(config::kTimezone);
  pet.setMood(Mood::kHappy, 2500);
  pet.hop();
  pet.say("I'm online!", 3000);
}

// Instant local reactions to touch, scaled by how Dexi is being touched. The
// brain sees the full gesture in /state and has the LLM respond to it too.
void reactWhileTouching(uint32_t now) {
  static bool purred = false;
  uint32_t held = touch.heldMs(now);
  if (!touch.held()) {
    purred = false;
    return;
  }
  if (held > 500) pet.setMood(Mood::kLove, 1500);  // keeps refreshing while held
  if (held > 1800 && !purred) {
    pet.say("purrrrrr...", 2500);
    purred = true;
  }
}

void reactToGesture(const Gesture& g) {
  uint32_t longest = 0, sumGap = 0;
  int n = min<int>(g.presses, Gesture::kMaxPresses);
  for (int i = 0; i < n; i++) longest = max<uint32_t>(longest, g.pressMs[i]);
  for (int i = 1; i < n; i++) sumGap += g.gapMs[i];
  uint32_t avgGap = n > 1 ? sumGap / (n - 1) : 0;

  if (longest > 1500) pet.setMood(Mood::kLove, 4000);  // cuddle
  else if (g.presses >= 5 && avgGap < 220) {             // drumming on its head
    pet.setMood(Mood::kAngry, 2500);
    pet.say("hey! stop poking!", 2500);
  } else if (g.presses >= 2) {
    pet.hop();
    pet.setMood(Mood::kExcited, 2500);
  } else if (longest > 400) pet.setMood(Mood::kCurious, 2500);  // a slow press
  else {
    pet.hop();
    pet.setMood(Mood::kHappy, 2500);  // a boop
  }
  Serial.printf("gesture #%lu: %u presses, longest %lu ms, avg gap %lu ms\n", g.id, g.presses,
                longest, avgGap);
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
  pets.begin();
  pet.setMood(Mood::kSleepy, 2000);  // waking up
  wifi.begin(config::kHostname, kNetworks, sizeof(kNetworks) / sizeof(kNetworks[0]));
  wifi.onFirstConnect(startServices);
  startWatchdog();
}

void loop() {
  static uint32_t lastFrame = 0;
  esp_task_wdt_reset();
  wifi.update();
  if (servicesUp) {
    server.handleClient();
    ArduinoOTA.handle();
  }
  if (updating) return;

  uint32_t now = millis();
  if (touch.update(now)) reactToGesture(touch.last());
  reactWhileTouching(now);
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
