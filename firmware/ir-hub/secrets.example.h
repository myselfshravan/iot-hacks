// Copy to secrets.h (git-ignored) and fill in. ESP32 is 2.4 GHz only.
// List every network the device may live on; it joins whichever is in range.
#pragma once
#define WIFI_NETWORKS                        \
  {                                          \
    {"home-town-ssid", "password"},          \
    {"bangalore-flat-ssid", "password"},     \
  }
