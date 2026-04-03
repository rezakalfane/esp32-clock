#include <ESPAsyncWebServer.h>  // ← must be absolute first
#include "clocknetwork.h"
#include "globals.h"
#include "config.h"
#include "webserver.h"
#include "led.h"
#include "mercury.h"
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <time.h>

// --- NTP sync task ---
void ntpTask(void* param) {
  for (;;) {
    ntpSyncing = true;
    Serial.println("[NTP] Syncing...");
    configTime(TZ_OFFSET, TZ_DST, NTP_SERVER);
    struct tm timeinfo;
    int retries = 0;
    while (!getLocalTime(&timeinfo) && retries < 20) {
      vTaskDelay(500 / portTICK_PERIOD_MS);
      retries++;
    }
    if (retries < 20) {
      DateTime ntpTime(
        timeinfo.tm_year + 1900,
        timeinfo.tm_mon + 1,
        timeinfo.tm_mday,
        timeinfo.tm_hour,
        timeinfo.tm_min,
        timeinfo.tm_sec
      );
      rtc.adjust(ntpTime);
      Serial.printf("[NTP] Synced: %02d:%02d:%02d\n",
                    timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);
    } else {
      Serial.println("[NTP] Sync failed");
    }
    ntpSyncing = false;
    vTaskDelay(NTP_INTERVAL / portTICK_PERIOD_MS);
  }
}

// --- Weather fetch task ---
void weatherTask(void* param) {
  for (;;) {
    weatherFetchInProgress = true;
    weatherFetchFailed     = false;
    Serial.println("[Weather] Fetching...");
    bool success = false;

    for (int attempt = 1; attempt <= WEATHER_MAX_RETRIES; attempt++) {
      HTTPClient http;
      String url = "https://api.open-meteo.com/v1/forecast?latitude=" + latitude +
                   "&longitude=" + longitude +
                   "&daily=weathercode&timezone=Europe%2FParis&forecast_days=1";
      http.begin(url);
      http.setTimeout(8000);
      int httpCode = http.GET();
      Serial.print("[Weather] HTTP: "); Serial.println(httpCode);

      if (httpCode == 200) {
        String payload = http.getString();
        StaticJsonDocument<512> doc;
        DeserializationError err = deserializeJson(doc, payload);
        if (!err) {
          weatherCode      = doc["daily"]["weathercode"][0];
          statusWeather    = true;
          lastWeatherFetch = millis();
          Serial.print("[Weather] Code: "); Serial.println(weatherCode);
          success = true;
          http.end();
          break;
        } else {
          Serial.print("[Weather] JSON error: "); Serial.println(err.c_str());
        }
      } else {
        Serial.println("[Weather] Error - retrying...");
      }
      http.end();
      vTaskDelay(3000 / portTICK_PERIOD_MS);
    }

    if (!success) { weatherFetchFailed = true; statusWeather = false; }
    weatherFetchInProgress = false;
    vTaskDelay(WEATHER_INTERVAL / portTICK_PERIOD_MS);
  }
}

// --- WiFi connection task ---
void wifiTask(void* param) {
  Serial.println("[WiFi] Connecting...");
  wifiConnecting = true;
  WiFi.begin(wifiSSID.c_str(), wifiPassword.c_str());

  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < WIFI_MAX_ATTEMPTS) {
    Serial.print("[WiFi] Attempt "); Serial.print(attempts + 1);
    Serial.print(" - Status: "); Serial.println(WiFi.status());
    vTaskDelay(500 / portTICK_PERIOD_MS);
    attempts++;
  }

  statusWiFi     = (WiFi.status() == WL_CONNECTED);
  wifiConnecting = false;

  if (statusWiFi) {
    Serial.print("[WiFi] Connected! IP: "); Serial.println(WiFi.localIP());
    startWebServer();
    xTaskCreatePinnedToCore(ntpTask,     "ntpTask",     4096, NULL, 1, NULL, 0);
    xTaskCreatePinnedToCore(weatherTask, "weatherTask", 8192, NULL, 1, NULL, 0);
  } else {
    Serial.println("[WiFi] FAILED - offline mode");
  }
  vTaskDelete(NULL);
}

// --- SSE push to web clients ---
void pushSSE() {
  clientConnected = (statusWiFi && events.count() > 0);
  if (!clientConnected) return;

  DateTime now   = rtc.now();
  float temp     = rtc.getTemperature();
  bool mercuryOn = readMercury();

  // Send true UTC so browser applies its own timezone
  uint32_t epoch = now.unixtime() - TZ_OFFSET;

  char dateBuf[16];
  snprintf(dateBuf, sizeof(dateBuf), "%s %02d/%02d/%04d",
           daysOfWeek[now.dayOfTheWeek()], now.day(), now.month(), now.year());

  int scheduledBrightness = computeLedBrightness(now.hour(), now.minute(), now.second());
  int ledPct = mercuryOn ? (currentBrightness * 100) / 255
                         : (scheduledBrightness * 100) / 255;

  String json = "{";
  json += "\"epoch\":"   + String(epoch) + ",";
  json += "\"date\":\""  + String(dateBuf) + "\",";
  json += "\"temp\":"    + String((int)round(temp)) + ",";
  json += "\"wifi\":"    + String(statusWiFi           ? "true" : "false") + ",";
  json += "\"wcode\":"   + String(weatherCode) + ",";
  json += "\"led\":"     + String(ledPct) + ",";
  json += "\"mercury\":" + String(mercuryOn             ? "true" : "false") + ",";
  json += "\"ntp\":"     + String(ntpSyncing            ? "true" : "false");
  json += "}";

  events.send(json.c_str(), "state", millis());
}
