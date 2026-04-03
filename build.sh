#!/bin/bash

PORT=${1:-/dev/cu.usbmodem2101}
FQBN="esp32:esp32:esp32c3"
SKETCH_DIR="/tmp/clock_build"
SKETCH="$SKETCH_DIR/clock_build.ino"
SRC="$HOME/Workspaces/ESP32-C3_MINI_V1/clock"

echo "==> Merging..."
rm -rf "$SKETCH_DIR" && mkdir -p "$SKETCH_DIR"
cp "$SRC"/*.h "$SKETCH_DIR"/

cat > "$SKETCH" << 'SKETCHEOF'
#include <Wire.h>
#include <RTClib.h>
#include <U8g2lib.h>
#include <math.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <Preferences.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <time.h>
#include "config.h"
#include "webserver_html.h"

SKETCHEOF

for f in clock clockprefs led mercury display clocknetwork webserver; do
  echo "// ===== $f ====" >> "$SKETCH"
  if [ "$f" = "clock" ]; then
    grep -Ev '^#include' "$SRC/clock.ino" >> "$SKETCH"
  else
    grep -Ev '^#include' "$SRC/${f}.cpp" >> "$SKETCH"
  fi
  echo "" >> "$SKETCH"
done

echo "==> Compiling..."
arduino-cli compile --fqbn "$FQBN" "$SKETCH_DIR"

if [ $? -eq 0 ]; then
  echo "==> Success! Press ENTER to upload to $PORT or Ctrl+C to cancel"
  read
  arduino-cli upload -p "$PORT" --fqbn "$FQBN" "$SKETCH_DIR"
else
  echo "==> Failed"
  exit 1
fi
