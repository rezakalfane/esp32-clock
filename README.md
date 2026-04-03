# Horloge ESP32-C3 — Smart Wake-Up Clock

A smart alarm clock built on an ESP32-C3 Super Mini with a real-time clock, OLED display, progressive LED wake-up light, weather forecast, mercury switch control, and a live web configuration interface.

---

## Features

- **Real-time clock** — DS3231 RTC module with battery backup
- **OLED display** — 128×32 showing time, temperature, WiFi status, weather icon
- **Progressive LED** — gradually ramps from 0% to 100% brightness over a configurable window, then turns off
- **Mercury switch** — physical on/off; tilting activates the display and LED schedule
- **Weather forecast** — fetches daily weather from Open-Meteo (no API key needed), displayed as icon
- **NTP sync** — automatically corrects RTC time after WiFi connects, re-syncs hourly
- **Web UI** — live dashboard + settings page served directly from the ESP32
- **AP fallback** — if WiFi fails, device creates a `ESP32-Clock` access point so settings can be updated from a phone
- **Persistent settings** — all parameters saved to NVS flash, survive power cycles
- **Reset to defaults** — tilt mercury switch 3× ON within 5 seconds to factory reset

---

## Hardware

### Components

| Component | Description |
|---|---|
| ESP32-C3 Super Mini | Main microcontroller |
| DS3231 / AT24C32 module | I2C real-time clock + EEPROM |
| SSD1306 128×32 OLED | I2C display |
| Mercury switch | Physical tilt on/off switch |
| LED | Wake-up light (any colour) |
| 220Ω resistor | LED current limiting |

### Wiring

#### I2C Bus (shared by RTC + OLED)

| Signal | ESP32-C3 Pin |
|---|---|
| SDA | GPIO 1 |
| SCL | GPIO 2 |
| VCC | 3.3V |
| GND | GND |

#### LED

```
GPIO10 ──── 220Ω ──── LED(+) ──── LED(-) ──── GND
```

#### Mercury Switch

```
GPIO7 ──── Mercury Switch ──── GND
```
Uses `INPUT_PULLUP` — switch closed = LOW = ON.

---

## Software Setup

### Arduino IDE Board Settings

| Setting | Value |
|---|---|
| Board | `ESP32C3 Dev Module` or `Nologo ESP32C3 Super Mini` |
| USB CDC On Boot | `Enabled` |
| Upload Speed | `921600` |

### Required Libraries

Install via **Sketch → Include Library → Manage Libraries**:

| Library | Author |
|---|---|
| RTClib | Adafruit |
| U8g2 | oliver |
| ArduinoJson | Benoit Blanchon |
| AsyncTCP | ESP32Async |
| ESP Async WebServer | ESP32Async |

### First Upload

1. Open `clock.ino` in Arduino IDE
2. Set your WiFi credentials in `clock.ino`:
   ```cpp
   String wifiSSID     = "YOUR_WIFI_SSID";
   String wifiPassword = "YOUR_WIFI_PASSWORD";
   ```
3. Set your location coordinates in `clock.ino`:
   ```cpp
   String latitude  = "48.218";
   String longitude = "-1.754";
   ```
4. Upload and open Serial Monitor at 115200 baud
5. Note the IP address shown after WiFi connects
6. Open `http://<IP>` in your browser

---

## File Structure

```
clock/
├── clock.ino           Main sketch — setup(), loop(), all global definitions
├── config.h            All #defines: pins, timing, thresholds
├── globals.h           Extern declarations for all shared variables
├── preferences.cpp/h   NVS load, save, reset to defaults
├── led.cpp/h           LED brightness computation (ramp schedule)
├── mercury.cpp/h       Debounced switch read + 3x reset detection
├── display.cpp/h       OLED rendering — bitmaps, icons, layout
├── network.cpp/h       WiFi task, NTP task, weather task, SSE push
├── webserver.cpp/h     HTTP routes, HTML template rendering
└── webserver_html.h    Full web UI as PROGMEM HTML string
```

---

## OLED Display Layout

```
┌────────────────────────────────┐
│  08:26:42    [wifi]  19°C      │  ← time + WiFi icon + temperature
│─────────────────────           │  ← divider (shorter than full width)
│  JEU 02/04/2026   T/C  ⛅      │  ← date + NTP/client indicator + weather icon
└────────────────────────────────┘
```

### Icons

| Icon | Meaning |
|---|---|
| WiFi arcs (animated) | Connecting to WiFi |
| Full WiFi arcs | Connected |
| WiFi arcs + X | No WiFi / connection failed |
| Round spinner (bottom right) | Fetching weather |
| Weather icon (bottom right) | Weather loaded |
| Blinking `T` (bottom right) | NTP time sync in progress |
| `C` (bottom right) | Web client connected — persists while page is open; alternates with `T` if both are active |
| `Paramètres enregistrés` (bottom row, 2 s) | Confirmation shown on display after saving settings from the web UI |
| `AP` (WiFi area) | Device is running as an Access Point (no WiFi available) |
| `Pas de WiFi...` (bottom row, 2 s) | Shown briefly while WiFi connection has failed, before AP starts |
| `ESP32-Clock  192.168.4.1` (bottom row, 3 s) | AP network info shown after AP starts |

---

## LED Schedule

The LED ramps from 0% to 100% brightness between `Début rampe` and `Pleine intensité`, stays at full brightness until `Extinction`, then turns off until the next day.

```
Time:        06:00    06:15    06:20
              |        |        |
Brightness:   0% ──── 100% ──── OFF
              ramp     full    off
```

The schedule only runs on **active days** (configurable per day of week).

When the mercury switch is turned off mid-ramp and turned back on, the LED resets to 0% and waits for the next ramp window.

---

## Web Interface

Access at `http://<device-IP>` once connected to WiFi.

### Live Dashboard

- **Clock** — live seconds-accurate time (synced via epoch offset, no lag)
- **Mercury badge** — teal `Actif` when switch is ON, orange `Veille` when OFF
- **T badge** — blinks during NTP sync
- **Temperature** — from DS3231 internal sensor
- **WiFi** — green dot connected, red dot disconnected
- **Météo** — weather label with emoji (fetched from Open-Meteo)
- **LED bar** — shows current brightness % (or scheduled brightness when switch is off)

### Settings

| Section | Parameters |
|---|---|
| Jours actifs | Toggle days LUN–DIM |
| Programme LED | Début rampe, Pleine intensité, Extinction |
| Localisation | Latitude / Longitude for weather |
| WiFi | SSID + password (password never displayed) |

The **Enregistrer** button is disabled when the device is disconnected.

### Reconnection

The web page automatically reconnects after device reboot — heartbeat timeout of 4 seconds triggers a new SSE connection attempt.

---

## Reset to Factory Defaults

To wipe all saved settings and reboot with defaults:

1. While the device is **running** (not during boot)
2. Tilt the mercury switch **ON → OFF → ON → OFF → ON** (3 complete ON pulses)
3. Do this within **5 seconds**
4. OLED shows `Réinitialisation des paramètres...` then `Redémarrage...`
5. Device reboots with default settings

> **Note:** WiFi credentials are also reset — you will need to set them again via the web UI after reconnecting to your network. If WiFi fails, the clock still works offline (time, LED schedule, OLED all function without network).

---

## Architecture

### FreeRTOS Tasks

| Task | Core | Stack | Description |
|---|---|---|---|
| `loop()` (Arduino) | Core 1 | — | Display, LED, mercury, SSE push |
| `wifiTask` | Core 0 | 8192 | WiFi connection, then spawns below |
| `ntpTask` | Core 0 | 4096 | NTP sync every hour |
| `weatherTask` | Core 0 | 8192 | Weather fetch every hour |

Network tasks run on Core 0 so the display on Core 1 never freezes during HTTP requests.

### Persistent Storage (NVS)

All settings are stored in the `clock` NVS namespace:

| Key | Type | Default |
|---|---|---|
| `ssid` | String | `YOUR_WIFI_SSID` |
| `pass` | String | `YOUR_WIFI_PASSWORD` |
| `lat` | String | `48.218` |
| `lon` | String | `-1.754` |
| `rampStartH/M` | Int | `6:00` |
| `rampEndH/M` | Int | `6:15` |
| `ledOffH/M` | Int | `6:20` |
| `activeDays` | Int | `0b0111110` (LUN-VEN) |

### SSE Payload

The server pushes a JSON event every second:

```json
{
  "epoch": 1743668802,
  "date": "JEU 02/04/2026",
  "temp": 19,
  "wifi": true,
  "wcode": 53,
  "led": 42,
  "mercury": true,
  "ntp": false
}
```

The `epoch` is true UTC (RTC local time minus TZ_OFFSET) so the browser applies its own timezone automatically.

---

## Timezone

France uses **CEST (UTC+2)** in summer and **CET (UTC+1)** in winter.

In `config.h`:
```cpp
#define TZ_OFFSET  7200   // UTC+2 summer (CEST)
#define TZ_DST     0
```

Change to `3600` in winter (CET).

---

## AP Fallback Mode

If the device cannot connect to the configured WiFi network, it automatically starts a fallback Access Point so you can update the credentials from a phone.

### Display sequence on WiFi failure

| Duration | WiFi area | Bottom row |
|---|---|---|
| 2 s | Crossed WiFi icon | `Pas de WiFi...` |
| 3 s | `AP` | `ESP32-Clock  192.168.4.1` |
| Ongoing | `AP` | Normal date + icons |

### Connecting and reconfiguring

1. On your phone, connect to WiFi network **`ESP32-Clock`** (password: `clock1234`)
2. Open **`http://192.168.4.1`** in your browser
3. Enter the correct WiFi SSID and password, then tap **Enregistrer**
4. The device shows the 2-second save confirmation then restarts automatically to connect with the new credentials

> Weather and NTP are unavailable in AP mode (no internet). The clock, LED schedule, and mercury switch all work normally.

---

## Troubleshooting

### OLED shows `RTC NOT FOUND`
- Check SDA → GPIO1, SCL → GPIO2
- Check VCC → 3.3V, GND → GND
- Run I2C scanner to verify addresses (should see 0x57 and 0x68)

### Time is wrong after boot
- NTP syncs automatically after WiFi connects
- If no WiFi, time comes from RTC battery — check battery

### LED stays off / doesn't ramp
- Check mercury switch is ON (closed)
- Check current time is within ramp window
- Check today is an active day in settings
- Serial monitor will show `[LED] Day active: 0` if day is inactive

### WiFi never connects
- Check SSID/password in settings
- ESP32-C3 only supports 2.4GHz networks
- Trigger factory reset (3x ON) and re-enter credentials via web UI

### Web page shows `Déconnecté` permanently
- Check device is powered and Serial shows `[WiFi] Connected`
- Try navigating directly to `http://<IP>`
- The page auto-reconnects every 4 seconds — wait 10 seconds after reboot

### Weather shows `Chargement...` permanently
- Check HTTP -1 errors in serial (timeout) — retry is automatic
- Check latitude/longitude are valid decimal coordinates
- Open-Meteo requires internet access — check WiFi signal strength

### Mercury switch triggers reset accidentally
- Increase `RESET_DEBOUNCE_MS` in `config.h` (currently 80ms)
- Increase `MERCURY_DEBOUNCE_MS` (currently 150ms)
- The reset requires 3 complete ON pulses within 5 seconds — normal use won't trigger it

---

## Weather Codes (WMO)

| Code | Icon | Description |
|---|---|---|
| 0 | ☀️ | Clear sky |
| 1–2 | ⛅ | Partly cloudy |
| 3 | ☁️ | Overcast |
| 51–67 | 🌧️ | Drizzle / Rain |
| 71–77 | ❄️ | Snow |
| 80–82 | 🌦️ | Rain showers |
| 95–99 | ⚡ | Thunderstorm |

---

## Credits

- Weather data: [Open-Meteo](https://open-meteo.com) (free, no API key)
- RTC library: [Adafruit RTClib](https://github.com/adafruit/RTClib)
- Display library: [U8g2](https://github.com/olikraus/u8g2)
- Web server: [ESP Async WebServer](https://github.com/ESP32Async/ESPAsyncWebServer)
