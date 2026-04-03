#pragma once

// --- I2C ---
#define SDA_PIN        1
#define SCL_PIN        2

// --- Hardware pins ---
#define LED_PIN        10
#define MERCURY_PIN    7

// --- LED PWM ---
#define PWM_FREQ       5000
#define PWM_RESOLUTION 8

// --- NTP ---
#define NTP_SERVER     "pool.ntp.org"
#define TZ_OFFSET      7200   // UTC+2 CEST (France summer)
#define TZ_DST         0

// --- Network ---
#define WIFI_MAX_ATTEMPTS    50
#define WEATHER_MAX_RETRIES  3
#define WEATHER_INTERVAL     3600000UL  // 1 hour
#define NTP_INTERVAL         3600000UL  // 1 hour

// --- SSE ---
#define SSE_INTERVAL         1000UL     // 1 second

// --- Mercury debounce ---
#define MERCURY_DEBOUNCE_MS  150

// --- Reset detection ---
#define RESET_WINDOW_MS      5000
#define RESET_DEBOUNCE_MS    80
#define RESET_ON_TARGET      3

// --- Fallback Access Point ---
#define AP_SSID "ESP32-Clock"
#define AP_PASS "clock1234"

// --- OLED icons position ---
#define WIFI_ICON_W  16
#define WIFI_ICON_H  11
#define WIFI_ICON_Y  3
#define ICON_X       110
#define ICON_Y       21
#define NTP_ICON_X   97
#define NTP_ICON_Y   21
