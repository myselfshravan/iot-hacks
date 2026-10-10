// Wi-Fi projector remote: phone web page + Alexa, sending the NEC codes
// captured in captures/projector-remote.md over the IR LED.
//
//   Phone:  http://projector.local  (or the IP printed on serial)
//   Alexa:  "Alexa, discover devices", then "Alexa, turn on/off the projector".
//           Espalexa emulates a Philips Hue bridge on the LAN, so no skill,
//           cloud account or Home Assistant is needed.
//
// Wiring: same as 06_ir_tx (see docs/wiring.md). Wi-Fi credentials go in
// secrets.h (copy secrets.example.h).

#include <Arduino.h>
#include <ESPmDNS.h>
#include <Espalexa.h>
#include <IRrecv.h>
#include <IRremoteESP8266.h>
#include <IRsend.h>
#include <IRutils.h>
#include <WebServer.h>
#include <WiFi.h>

#include "secrets.h"

constexpr char kHostname[] = "projector";
constexpr uint16_t kRecvPin = 14;
constexpr uint8_t kRxPowerPin = 27;
constexpr uint16_t kSendPin = 26;

struct Button {
  const char* id;
  const char* label;
  uint32_t code;
};

constexpr Button kButtons[] = {
    {"power", "Power", 0x61D650AF},  {"up", "Up", 0x61D6AA55},
    {"down", "Down", 0x61D618E7},    {"left", "Left", 0x61D6A857},
    {"right", "Right", 0x61D69A65},  {"volup", "Vol +", 0x61D6708F},
    {"voldown", "Vol -", 0x61D612ED}, {"back", "Back", 0x61D6728D},
    {"home", "Home", 0x61D6CA35},    {"ok", "OK", 0x61D628D7},
    {"s", "S", 0x61D6D02F},
    {"settings", "Settings", 0x61D6F807},
};

IRsend irsend(kSendPin);
IRrecv irrecv(kRecvPin, 1024, 50, true);
decode_results results;
WebServer server(80);
Espalexa espalexa;

String lastSeen = "-";  // last IR code heard by the receiver (any source)

const Button* findButton(const String& id) {
  for (const auto& b : kButtons)
    if (id == b.id) return &b;
  return nullptr;
}

const Button* findCode(uint32_t code) {
  for (const auto& b : kButtons)
    if (b.code == code) return &b;
  return nullptr;
}

void press(const Button& b, const char* source) {
  irsend.sendNEC(b.code, 32);
  Serial.printf("TX %-8s 0x%08X  (from %s)\n", b.id, b.code, source);
}

const char kPage[] PROGMEM = R"HTML(<!doctype html>
<html><head><meta charset="utf-8">
<meta name="viewport" content="width=device-width,initial-scale=1">
<title>Projector</title>
<style>
:root{--bg:#111;--fg:#eee;--key:#2a2a2a;--hot:#c0392b;--acc:#3d7eff}
body{margin:0;background:var(--bg);color:var(--fg);font:16px system-ui;
display:flex;flex-direction:column;align-items:center;padding:24px 16px}
h1{font-size:20px;margin:0 0 20px}
.grid{display:grid;grid-template-columns:repeat(3,84px);gap:12px}
button{height:64px;border:0;border-radius:14px;background:var(--key);
color:var(--fg);font-size:16px;touch-action:manipulation}
button:active{background:var(--acc)}
#power{background:var(--hot)}
#ok{background:var(--acc)}
.gap{visibility:hidden}
#status{margin-top:24px;font-size:13px;opacity:.7;text-align:center}
</style></head><body>
<h1>Projector</h1>
<div class="grid">
<button id="power">Power</button><button class="gap"></button><button id="settings">Settings</button>
<button class="gap"></button><button id="up">&#9650;</button><button class="gap"></button>
<button id="left">&#9664;</button><button id="ok">OK</button><button id="right">&#9654;</button>
<button id="back">Back</button><button id="down">&#9660;</button><button id="home">Home</button>
<button id="voldown">Vol &minus;</button><button id="s">S</button><button id="volup">Vol +</button>
</div>
<div id="status">ready</div>
<script>
const st=document.getElementById('status');
document.querySelectorAll('button[id]').forEach(b=>b.onclick=async()=>{
  const r=await fetch('/send?b='+b.id).catch(()=>null);
  st.textContent=r&&r.ok?'sent '+b.id:'failed';
  if(navigator.vibrate)navigator.vibrate(15);
});
setInterval(async()=>{
  const r=await fetch('/last').catch(()=>null);
  if(r&&r.ok)st.title='last IR seen: '+await r.text();
},2000);
</script></body></html>)HTML";

void handleSend() {
  const Button* b = findButton(server.arg("b"));
  if (!b) {
    server.send(400, "text/plain", "unknown button");
    return;
  }
  press(*b, "web");
  server.send(200, "text/plain", "ok");
}

void onAlexaProjector(uint8_t brightness) {
  // The projector only has a power toggle, so on and off both send Power.
  Serial.printf("Alexa: projector %s\n", brightness ? "ON" : "OFF");
  press(*findButton("power"), "alexa");
}

void connectWifi() {
  WiFi.mode(WIFI_STA);
  WiFi.setHostname(kHostname);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.printf("Connecting to %s", WIFI_SSID);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print('.');
  }
  Serial.printf("\nWi-Fi OK  IP=%s  RSSI=%d dBm\n",
                WiFi.localIP().toString().c_str(), WiFi.RSSI());
}

void setup() {
  Serial.begin(115200);
  pinMode(kRxPowerPin, OUTPUT);
  digitalWrite(kRxPowerPin, HIGH);
  irsend.begin();
  irrecv.enableIRIn();

  connectWifi();
  if (MDNS.begin(kHostname)) MDNS.addService("http", "tcp", 80);

  server.on("/", HTTP_GET, [] { server.send_P(200, "text/html", kPage); });
  server.on("/send", HTTP_GET, handleSend);
  server.on("/last", HTTP_GET, [] { server.send(200, "text/plain", lastSeen); });
  server.onNotFound([] {
    if (!espalexa.handleAlexaApiCall(server.uri(), server.arg(0)))
      server.send(404, "text/plain", "Not found");
  });

  espalexa.addDevice("Projector", onAlexaProjector);
  espalexa.begin(&server);  // also starts the web server

  Serial.printf("READY  http://%s.local  or  http://%s\n", kHostname,
                WiFi.localIP().toString().c_str());
}

void loop() {
  espalexa.loop();  // handles both the web page and Alexa requests
  if (irrecv.decode(&results)) {
    const Button* b = findCode(results.value);
    lastSeen = b ? b->label : uint64ToString(results.value, 16);
    if (!results.repeat) Serial.printf("RX %s\n", lastSeen.c_str());
  }
  if (WiFi.status() != WL_CONNECTED) connectWifi();
}
