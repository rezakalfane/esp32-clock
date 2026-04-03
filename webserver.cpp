#include <ESPAsyncWebServer.h>  // ← must be absolute first
#include "webserver.h"
#include "webserver_html.h"
#include "globals.h"
#include "config.h"
#include "preferences.h"
#include "led.h"
#include <WiFi.h>

static String zeroPad(int v) {
  return v < 10 ? "0" + String(v) : String(v);
}

static String buildHTML() {
  String html = String(INDEX_HTML);
  html.replace("%IP%",           WiFi.localIP().toString());
  html.replace("%SSID%",         wifiSSID);
  html.replace("%LAT%",          latitude);
  html.replace("%LON%",          longitude);
  html.replace("%RAMP_START_H%", zeroPad(rampStartH));
  html.replace("%RAMP_START_M%", zeroPad(rampStartM));
  html.replace("%RAMP_END_H%",   zeroPad(rampEndH));
  html.replace("%RAMP_END_M%",   zeroPad(rampEndM));
  html.replace("%LED_OFF_H%",    zeroPad(ledOffH));
  html.replace("%LED_OFF_M%",    zeroPad(ledOffM));
  for (int i = 0; i <= 6; i++)
    html.replace("%DAY" + String(i) + "%", ((activeDays >> i) & 1) ? "checked" : "");
  return html;
}

void startWebServer() {
  server.on("/", HTTP_GET, [](AsyncWebServerRequest* req) {
    req->send(200, "text/html", buildHTML());
  });

  server.on("/save", HTTP_POST, [](AsyncWebServerRequest* req) {
    // WiFi: only update if both SSID and password provided
    String newSSID = req->hasParam("ssid", true) ? req->getParam("ssid", true)->value() : "";
    String newPass = req->hasParam("pass", true) ? req->getParam("pass", true)->value() : "";
    if (newSSID.length() > 0 && newPass.length() > 0) {
      wifiSSID = newSSID;
      wifiPassword = newPass;
      Serial.println("[Web] WiFi credentials updated");
    } else {
      Serial.println("[Web] WiFi credentials unchanged");
    }

    if (req->hasParam("lat",        true)) latitude   = req->getParam("lat",        true)->value();
    if (req->hasParam("lon",        true)) longitude  = req->getParam("lon",        true)->value();
    if (req->hasParam("rampStartH", true)) rampStartH = req->getParam("rampStartH", true)->value().toInt();
    if (req->hasParam("rampStartM", true)) rampStartM = req->getParam("rampStartM", true)->value().toInt();
    if (req->hasParam("rampEndH",   true)) rampEndH   = req->getParam("rampEndH",   true)->value().toInt();
    if (req->hasParam("rampEndM",   true)) rampEndM   = req->getParam("rampEndM",   true)->value().toInt();
    if (req->hasParam("ledOffH",    true)) ledOffH    = req->getParam("ledOffH",    true)->value().toInt();
    if (req->hasParam("ledOffM",    true)) ledOffM    = req->getParam("ledOffM",    true)->value().toInt();

    int newDays = 0;
    for (int i = 0; i <= 6; i++)
      if (req->hasParam("day" + String(i), true)) newDays |= (1 << i);
    activeDays = newDays;

    savePreferences();
    saveFeedback      = true;
    saveFeedbackStart = millis();

    // Recompute LED cycle with new schedule
    DateTime now = rtc.now();
    int totalSeconds = now.hour() * 3600 + now.minute() * 60 + now.second();
    int offSeconds   = ledOffH * 3600 + ledOffM * 60;
    bool dayActive   = (activeDays >> now.dayOfTheWeek()) & 1;
    ledCycleComplete = (totalSeconds >= offSeconds) || !dayActive;
    Serial.println("[Web] Settings saved and applied");

    req->send(200, "text/plain", "OK");
  });

  events.onConnect([](AsyncEventSourceClient* client) {
    Serial.println("[SSE] Client connected");
    client->send("", NULL, millis(), 1000); // retry every 1s
    clientConnected = true;
  });

  server.addHandler(&events);
  server.begin();
  Serial.print("[Web] Server started at http://");
  Serial.println(WiFi.localIP());
}
