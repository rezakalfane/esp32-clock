#include <Arduino.h>
#include <Preferences.h>
#include "clockprefs.h"
#include "globals.h"

static Preferences prefs;

void loadPreferences() {
  prefs.begin("clock", false);
  wifiSSID     = prefs.getString("ssid",      wifiSSID);
  wifiPassword = prefs.getString("pass",      wifiPassword);
  latitude     = prefs.getString("lat",       latitude);
  longitude    = prefs.getString("lon",       longitude);
  rampStartH   = prefs.getInt("rampStartH",   rampStartH);
  rampStartM   = prefs.getInt("rampStartM",   rampStartM);
  rampEndH     = prefs.getInt("rampEndH",     rampEndH);
  rampEndM     = prefs.getInt("rampEndM",     rampEndM);
  ledOffH      = prefs.getInt("ledOffH",      ledOffH);
  ledOffM      = prefs.getInt("ledOffM",      ledOffM);
  activeDays   = prefs.getInt("activeDays",   activeDays);
  prefs.end();
  Serial.println("[Prefs] Loaded from NVS");
}

void savePreferences() {
  prefs.begin("clock", false);
  prefs.putString("ssid",      wifiSSID);
  prefs.putString("pass",      wifiPassword);
  prefs.putString("lat",       latitude);
  prefs.putString("lon",       longitude);
  prefs.putInt("rampStartH",   rampStartH);
  prefs.putInt("rampStartM",   rampStartM);
  prefs.putInt("rampEndH",     rampEndH);
  prefs.putInt("rampEndM",     rampEndM);
  prefs.putInt("ledOffH",      ledOffH);
  prefs.putInt("ledOffM",      ledOffM);
  prefs.putInt("activeDays",   activeDays);
  prefs.end();
  Serial.println("[Prefs] Saved to NVS");
}

void resetToDefaults() {
  Serial.println("[Prefs] Resetting to defaults...");
  prefs.begin("clock", false);
  prefs.clear();
  prefs.end();
  wifiSSID     = "YOUR_WIFI_SSID";
  wifiPassword = "YOUR_WIFI_PASSWORD";
  latitude     = "48.218";
  longitude    = "-1.754";
  rampStartH   = 6;
  rampStartM   = 0;
  rampEndH     = 6;
  rampEndM     = 15;
  ledOffH      = 6;
  ledOffM      = 20;
  activeDays   = 0b0111110;
  Serial.println("[Prefs] Reset complete");
}
