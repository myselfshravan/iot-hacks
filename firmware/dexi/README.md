# Dexi: AI desk pet (ESP32 #3), plan

A character on the 1.8" TFT that reacts to the room and talks to an AI service.

## Parts
| Part | Need | Why |
|---|---|---|
| ESP32 (3rd board) | required | Brain and Wi-Fi |
| 1.8" ST7735 TFT, 128×160, SPI | on hand | Face and animations |
| TTP223 touch pad | cheap, recommended | You can pet it |
| INMP441 I2S mic | optional | Hears claps and voice (needs soldering) |
| MAX98357A I2S amp + small speaker | optional | Chirps and speech |
| DHT (or read it from the Kelvin) | optional | Reacts to room heat |
| LD2410C mmWave presence sensor | optional | Sleeps when nobody is there |

## Architecture
```
Service (Mac now, cloud later; uses the Claude API) ──Wi-Fi──► ESP32 ──SPI──► TFT
  decides mood, writes lines, may push new sprites        animates locally, works offline
```
- The ESP32 owns smooth animation.
- The service sends small commands such as `mood: sleepy` or `say: "..."`.
- It can read the Kelvin's `/state` to react to the AC: shiver when it's cold, sweat when it's hot.

## First milestone
TFT bring-up, then one idle animation, then moods switched over HTTP.
