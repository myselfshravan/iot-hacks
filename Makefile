# Build/flash any sketch: make flash APP=ir-hub  (or APP=experiments/06_ir_tx)
FQBN ?= esp32:esp32:esp32
PORT ?= /dev/cu.usbserial-0001
APP  ?= ir-hub
LIBS := --libraries libraries
DIR  := $(if $(wildcard firmware/$(APP)),firmware/$(APP),$(APP))

.PHONY: build flash ota monitor log check intellisense lights

build:
	python3 tools/embed_web.py
	arduino-cli compile -b $(FQBN) $(LIBS) $(DIR)

flash: build
	arduino-cli upload -b $(FQBN) -p $(PORT) $(DIR)

# Flash over Wi-Fi (sketch must include ArduinoOTA): make ota APP=dexi [HOST=dexi.local]
ESPOTA := $(lastword $(wildcard $(HOME)/Library/Arduino15/packages/esp32/hardware/esp32/*/tools/espota.py))
ota: build
	python3 $(ESPOTA) -i $$(python3 -c "import socket;print(socket.gethostbyname('$(or $(HOST),$(APP).local)'))") \
	  -f $$(ls -t $(HOME)/Library/Caches/arduino/sketches/*/$(notdir $(DIR)).ino.bin | head -1)

monitor:
	arduino-cli monitor -p $(PORT) -c baudrate=115200

# Save a serial session: make log NAME=ac-02 SECONDS=60
log:
	uv run --with pyserial python tools/serial_log.py --out captures/raw/$(NAME).log --seconds $(or $(SECONDS),30)

# Compile every sketch (firmware + experiments).
check:
	@python3 tools/embed_web.py
	@for d in firmware/*/ experiments/*/; do \
	  ls $$d*.ino >/dev/null 2>&1 || continue; \
	  echo "== $$d"; arduino-cli compile -b $(FQBN) $(LIBS) $$d >/dev/null || exit 1; \
	done; echo "all sketches build"

intellisense:
	python3 tools/vscode_intellisense.py

# Monitor LED strip: `make lights` (status), DO=screen, DO=off, DO=candle,
# DO="chase ff0000", DO=start / DO=stop (HyperHDR on/off), DO=restart (stuck). See tools/lights.sh.
lights:
	@DEXI=$(or $(HOST),dexi.local) tools/lights.sh $(DO)
