// WAVESS AC controller: synthesises WAVESS IR frames from a state model and
// tracks the physical remote. Protocol: captures/wavess-protocol.md.
//
//   Phone:  http://ac.local  (or the IP printed on serial)
//   API:    GET /state                       -> JSON
//           GET /set?power=1&mode=cool&temp=24&fan=auto&swing=1  (any subset)
//   Alexa:  "Alexa, discover devices", then "turn on/off the AC",
//           "set the AC to 24" (Alexa sends 24 %, mapped to 24 °C).
//   Serial: "set mode=cool temp=24 fan=low swing=1 power=1", "state"
//
// The receiver also decodes WAVESS frames from the physical remote, so the
// state follows whoever used the remote last (if the receiver saw it).
// Wiring: same as 06_ir_tx (docs/wiring.md). Wi-Fi in secrets.h.

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

constexpr char kHostname[] = "ac";
constexpr uint16_t kRecvPin = 14;
constexpr uint8_t kRxPowerPin = 27;
constexpr uint16_t kSendPin = 26;

// Timings measured from the WAVESS remote (us).
constexpr uint16_t kHdrMark = 9025, kHdrSpace = 4530, kBitMark = 530;
constexpr uint16_t kOneSpace = 1660, kZeroSpace = 595;
constexpr uint16_t kBits = 40;

const char* const kModes[] = {"cool", "dry", "fan", "heat", "auto"};
const char* const kFans[] = {"auto", "high", "medium", "low"};

struct AcState {
  bool power = false;
  uint8_t mode = 0;   // index into kModes
  uint8_t temp = 24;  // 16-30
  uint8_t fan = 0;    // index into kFans
  bool swing = true;
  const char* source = "boot";
};

AcState ac;
IRsend irsend(kSendPin);
IRrecv irrecv(kRecvPin, 1024, 50, true);
decode_results results;
WebServer server(80);
Espalexa espalexa;
EspalexaDevice* alexaAc = nullptr;

// Nibbles that never changed in any capture.
constexpr uint8_t kConstNibbles[10] = {0, 0, 0, 1, 0, 0, 0, 1, 12, 0};

uint64_t encode(const AcState& s) {
  uint8_t n[10];
  memcpy(n, kConstNibbles, sizeof(n));
  n[0] = (s.power ? 8 : 0) | s.mode;
  n[2] = constrain(s.temp, 16, 30) - 15;
  n[4] = (s.swing ? 8 : 0) | s.fan;
  uint64_t bits = 0;
  for (int i = 0; i < 10; i++) bits |= (uint64_t)(n[i] & 0xF) << (4 * i);
  return bits;  // bit 0 is transmitted first
}

bool decode(uint64_t bits, AcState* s) {
  uint8_t n[10];
  for (int i = 0; i < 10; i++) n[i] = (bits >> (4 * i)) & 0xF;
  if (n[3] != 1 || n[7] != 1 || n[8] != 12) return false;  // not a WAVESS frame
  if ((n[0] & 7) > 4 || n[2] < 1 || n[2] > 15) return false;
  s->power = n[0] & 8;
  s->mode = n[0] & 7;
  s->temp = n[2] + 15;
  s->fan = n[4] & 3;
  s->swing = n[4] & 8;
  return true;
}

// Pulls 40 bits out of a raw capture (header + 40 mark/space pairs).
bool rawToBits(const decode_results& r, uint64_t* out) {
  uint16_t len = r.rawlen;
  if (len < 3 + 2 * kBits) return false;
  uint32_t hm = r.rawbuf[1] * kRawTick, hs = r.rawbuf[2] * kRawTick;
  if (hm < 7000 || hm > 11000 || hs < 3500 || hs > 5500) return false;
  uint64_t bits = 0;
  for (int i = 0; i < kBits; i++) {
    uint32_t space = r.rawbuf[4 + 2 * i] * kRawTick;
    if (space > 1100) bits |= 1ULL << i;
  }
  *out = bits;
  return true;
}

String stateJson() {
  return String("{\"power\":") + (ac.power ? "true" : "false") +
         ",\"mode\":\"" + kModes[ac.mode] + "\",\"temp\":" + ac.temp +
         ",\"fan\":\"" + kFans[ac.fan] + "\",\"swing\":" +
         (ac.swing ? "true" : "false") + ",\"source\":\"" + ac.source + "\"}";
}

void transmit(const char* source) {
  ac.source = source;
  uint64_t bits = encode(ac);
  irrecv.pause();
  irsend.sendGeneric(kHdrMark, kHdrSpace, kBitMark, kOneSpace, kBitMark,
                     kZeroSpace, kBitMark, 20000, bits, kBits, 38, false, 0, 50);
  irrecv.resume();
  Serial.printf("TX 0x%010llX  %s\n", bits, stateJson().c_str());
  if (alexaAc) {
    alexaAc->setState(ac.power);
    alexaAc->setPercent(ac.temp);
  }
}

int indexOf(const char* const* list, int n, const String& v) {
  for (int i = 0; i < n; i++)
    if (v.equalsIgnoreCase(list[i])) return i;
  return -1;
}

// Applies key=value; returns false on a bad key/value.
bool applyField(const String& k, const String& v) {
  if (k == "power") ac.power = (v == "1" || v == "on" || v == "true");
  else if (k == "swing") ac.swing = (v == "1" || v == "on" || v == "true");
  else if (k == "temp") {
    int t = v.toInt();
    if (t < 16 || t > 30) return false;
    ac.temp = t;
  } else if (k == "mode") {
    int i = indexOf(kModes, 5, v);
    if (i < 0) return false;
    ac.mode = i;
  } else if (k == "fan") {
    int i = indexOf(kFans, 4, v);
    if (i < 0) return false;
    ac.fan = i;
  } else return false;
  return true;
}

const char kPage[] PROGMEM = R"HTML(<!doctype html>
<html><head><meta charset="utf-8">
<meta name="viewport" content="width=device-width,initial-scale=1">
<title>AC</title>
<style>
:root{--bg:#0e1116;--card:#1a1f27;--fg:#eef;--dim:#8a93a3;--acc:#3d9bff;--on:#2ecc71;--off:#555}
body{margin:0;background:var(--bg);color:var(--fg);font:16px system-ui;padding:24px 16px;
display:flex;flex-direction:column;align-items:center;gap:16px}
.card{background:var(--card);border-radius:20px;padding:20px;width:100%;max-width:360px;box-sizing:border-box}
#power{width:100%;height:64px;border:0;border-radius:16px;font-size:20px;color:#fff;background:var(--off)}
#power.on{background:var(--on)}
.temp{display:flex;align-items:center;justify-content:space-between}
.temp b{font-size:56px;font-weight:600}
.temp button{width:64px;height:64px;border-radius:50%;border:0;font-size:28px;background:#2a313c;color:var(--fg)}
.label{color:var(--dim);font-size:13px;margin:4px 0 8px}
.seg{display:flex;gap:6px;flex-wrap:wrap}
.seg button{flex:1;min-width:56px;height:44px;border:0;border-radius:12px;background:#2a313c;color:var(--fg)}
.seg button.sel{background:var(--acc)}
#status{color:var(--dim);font-size:12px}
</style></head><body>
<div class="card"><button id="power">Power</button></div>
<div class="card temp"><button data-dt="-1">&minus;</button><b id="temp">--</b><button data-dt="1">+</button></div>
<div class="card"><div class="label">Mode</div><div class="seg" id="mode"></div>
<div class="label">Fan</div><div class="seg" id="fan"></div>
<div class="label">Swing</div><div class="seg" id="swing"></div></div>
<div id="status">loading…</div>
<script>
const MODES=['cool','dry','fan','heat','auto'],FANS=['auto','low','medium','high'];
let s={};
function seg(id,opts,key){const el=document.getElementById(id);el.innerHTML='';
 opts.forEach(o=>{if(typeof o!=='object')o={v:o,label:o};
 const b=document.createElement('button');b.textContent=o.label;
 if(String(s[key])===String(o.v))b.className='sel';
 b.onclick=()=>set({[key]:o.v});el.appendChild(b);});}
function render(){power.className=s.power?'on':'';power.textContent=s.power?'ON':'OFF';
 temp.textContent=s.temp+'°';seg('mode',MODES,'mode');seg('fan',FANS,'fan');
 seg('swing',[{v:true,label:'on'},{v:false,label:'off'}],'swing');
 status.textContent='last change: '+s.source;}
async function set(p){const q=new URLSearchParams(Object.entries(p).map(([k,v])=>[k,v===true?1:v===false?0:v]));
 const r=await fetch('/set?'+q).catch(()=>null);if(r&&r.ok){s=await r.json();render();}else status.textContent='failed';}
async function load(){const r=await fetch('/state').catch(()=>null);if(r&&r.ok){s=await r.json();render();}}
power.onclick=()=>set({power:!s.power});
document.querySelectorAll('[data-dt]').forEach(b=>b.onclick=()=>set({temp:Math.min(30,Math.max(16,s.temp+ +b.dataset.dt))}));
load();setInterval(load,3000);
</script></body></html>)HTML";

void handleSet() {
  for (int i = 0; i < server.args(); i++) {
    if (!applyField(server.argName(i), server.arg(i))) {
      server.send(400, "text/plain", "bad " + server.argName(i));
      return;
    }
  }
  transmit("app");
  server.send(200, "application/json", stateJson());
}

void onAlexa(EspalexaDevice* d) {
  if (!d) return;
  bool on = d->getValue() > 0;
  uint8_t pct = d->getPercent();
  ac.power = on;
  if (on && pct >= 16 && pct <= 30) ac.temp = pct;
  transmit("alexa");
}

void handleSerial(String line) {
  line.trim();
  if (line == "state") {
    Serial.println(stateJson());
    return;
  }
  if (!line.startsWith("set ")) {
    Serial.println("ERR: set k=v [k=v...] | state");
    return;
  }
  line = line.substring(4);
  while (line.length()) {
    int sp = line.indexOf(' ');
    String tok = sp < 0 ? line : line.substring(0, sp);
    line = sp < 0 ? "" : line.substring(sp + 1);
    int eq = tok.indexOf('=');
    if (eq < 0 || !applyField(tok.substring(0, eq), tok.substring(eq + 1))) {
      Serial.println("ERR bad field: " + tok);
      return;
    }
  }
  transmit("serial");
}

bool connectWifi(uint32_t timeoutMs) {
  WiFi.mode(WIFI_STA);
  WiFi.setHostname(kHostname);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.printf("Connecting to %s", WIFI_SSID);
  uint32_t start = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - start < timeoutMs) {
    delay(500);
    Serial.print('.');
  }
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("\nWi-Fi FAILED (serial control still works)");
    return false;
  }
  Serial.printf("\nWi-Fi OK  IP=%s  RSSI=%d dBm\n",
                WiFi.localIP().toString().c_str(), WiFi.RSSI());
  return true;
}

void setup() {
  Serial.begin(115200);
  pinMode(kRxPowerPin, OUTPUT);
  digitalWrite(kRxPowerPin, HIGH);
  irsend.begin();
  irrecv.enableIRIn();

  if (connectWifi(20000)) {
    if (MDNS.begin(kHostname)) MDNS.addService("http", "tcp", 80);
    server.on("/", HTTP_GET, [] { server.send_P(200, "text/html", kPage); });
    server.on("/state", HTTP_GET,
              [] { server.send(200, "application/json", stateJson()); });
    server.on("/set", HTTP_GET, handleSet);
    server.onNotFound([] {
      if (!espalexa.handleAlexaApiCall(server.uri(), server.arg(0)))
        server.send(404, "text/plain", "Not found");
    });
    alexaAc = new EspalexaDevice("AC", onAlexa, EspalexaDeviceType::dimmable, 0);
    espalexa.addDevice(alexaAc);
    espalexa.begin(&server);
    Serial.printf("READY  http://%s.local  or  http://%s\n", kHostname,
                  WiFi.localIP().toString().c_str());
  }
  Serial.println("Serial: set mode=cool temp=24 fan=low swing=1 power=1 | state");
}

void loop() {
  if (WiFi.status() == WL_CONNECTED) espalexa.loop();
  if (Serial.available()) handleSerial(Serial.readStringUntil('\n'));
  if (irrecv.decode(&results)) {
    uint64_t bits;
    AcState seen = ac;
    if (rawToBits(results, &bits) && decode(bits, &seen)) {
      seen.source = "remote";
      if (encode(seen) != encode(ac)) {  // ignore echoes of our own TX
        ac = seen;
        if (alexaAc) alexaAc->setState(ac.power);
        Serial.printf("RX remote 0x%010llX  %s\n", bits, stateJson().c_str());
      }
    }
  }
}
