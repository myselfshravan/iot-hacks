// Phase A bring-up: proves serial, GPIO and the Wi-Fi radio work.
// - Prints chip info over serial (115200 baud)
// - Blinks the onboard blue LED (GPIO 2 on 30-pin DevKit boards)
// - Scans nearby Wi-Fi networks every 10 s (no credentials needed)

#include <WiFi.h>

constexpr int kLedPin = 2;
constexpr uint32_t kScanIntervalMs = 10000;

uint32_t lastScan = 0;

void scanWifi() {
  Serial.println("\n--- Wi-Fi scan ---");
  int n = WiFi.scanNetworks();
  if (n <= 0) {
    Serial.println("No networks found");
    return;
  }
  for (int i = 0; i < n; i++) {
    Serial.printf("%2d  %4d dBm  ch%2d  %s\n", i + 1, WiFi.RSSI(i),
                  WiFi.channel(i), WiFi.SSID(i).c_str());
  }
  WiFi.scanDelete();
}

void setup() {
  Serial.begin(115200);
  delay(500);
  pinMode(kLedPin, OUTPUT);

  Serial.println("\n=== esp32-hacks bring-up ===");
  Serial.printf("Chip: %s rev %d, %d cores @ %d MHz\n", ESP.getChipModel(),
                ESP.getChipRevision(), ESP.getChipCores(), ESP.getCpuFreqMHz());
  Serial.printf("Flash: %u MB, free heap: %u bytes\n",
                ESP.getFlashChipSize() / (1024 * 1024), ESP.getFreeHeap());

  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  Serial.printf("MAC: %s\n", WiFi.macAddress().c_str());
  scanWifi();
  lastScan = millis();
}

void loop() {
  digitalWrite(kLedPin, (millis() / 500) % 2);  // 1 Hz blink

  if (millis() - lastScan >= kScanIntervalMs) {
    scanWifi();
    lastScan = millis();
  }
}
