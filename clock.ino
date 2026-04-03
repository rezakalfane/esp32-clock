#include <ESPAsyncWebServer.h>  // ← must be absolute first
#include <Wire.h>
#include <RTClib.h>
#include <U8g2lib.h>
#include <WiFi.h>
#include <Preferences.h>
#include <math.h>
extern "C" void esp_brownout_disable(void);

#include "config.h"
#include "globals.h"
#include "clockprefs.h"
#include "display.h"
#include "led.h"
#include "mercury.h"
#include "clocknetwork.h"
#include "webserver.h"

// --- Hardware instances ---
RTC_DS3231 rtc;
AsyncWebServer server(80);
AsyncEventSource events("/events");
U8G2_SSD1306_128X32_UNIVISION_F_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE, SCL_PIN, SDA_PIN);

// --- Day names ---
const char* daysOfWeek[] = {
  "DIM", "LUN", "MAR", "MER", "JEU", "VEN", "SAM"
};

// --- Settings ---
String  wifiSSID     = "YOUR_WIFI_SSID";
String  wifiPassword = "YOUR_WIFI_PASSWORD";
String  latitude     = "48.218";
String  longitude    = "-1.754";
int     rampStartH   = 6;
int     rampStartM   = 0;
int     rampEndH     = 6;
int     rampEndM     = 15;
int     ledOffH      = 6;
int     ledOffM      = 20;
int     activeDays   = 0b0111110;  // LUN-VEN active by default

// --- Runtime state ---
bool ledCycleComplete  = false;
bool lastSwitchState   = false;
bool statusRTC         = false;
bool statusWiFi        = false;
bool statusWeather     = false;
bool weatherFetchFailed = false;
int  weatherCode       = -1;
int  currentBrightness = 0;

// --- Display animation ---
int  wifiSpinFrame    = 0;
int  weatherSpinFrame = 0;
unsigned long lastWifiSpinUpdate    = 0;
unsigned long lastWeatherSpinUpdate = 0;
unsigned long lastNtpBlink          = 0;
bool ntpBlinkState = false;
bool          clientConnected    = false;
bool          apMode             = false;
unsigned long restartAt          = 0;
// --- Sliding notification ---
NotifState    notifState    = NOTIF_IDLE;
int           notifSlideY   = 11;
unsigned long notifLastStep = 0;
unsigned long notifShowUntil = 0;
unsigned long notifDuration  = 0;
char          notifText[32]  = "";
bool          notifSmallFont = false;
bool          notifIsUTF8    = false;

// --- Timing ---
unsigned long lastWeatherFetch = 0;
unsigned long lastSSEUpdate    = 0;

// --- Task flags ---
volatile bool wifiConnecting         = false;
volatile bool weatherFetchInProgress = false;
volatile bool ntpSyncing             = false;

// --- Mercury debounce ---
unsigned long lastMercuryDebounce = 0;
bool          debouncedMercury    = false;
bool          lastRawMercury      = false;

// --- Live reset detection ---
int           resetToggleCount   = 0;
int           resetOnCount       = 0;
unsigned long resetWindowStart   = 0;
unsigned long resetDebounceStart = 0;
bool          resetPendingChange = false;
bool          resetPendingState  = false;
bool          resetLastState     = false;
bool          resetInitialized   = false;

// ============================================================
void setup() {
  // Disable brownout detector — prevents reset loops when WiFi radio draws current spikes on battery
  esp_brownout_disable();

  Serial.begin(115200);
  // Non-blocking: wait up to 1s for Serial, then continue (battery/standalone mode)
  unsigned long serialStart = millis();
  while (!Serial && millis() - serialStart < 1000) delay(10);
  Serial.println("\n=== BOOT ===");

  // LED off immediately
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);
  ledcAttach(LED_PIN, PWM_FREQ, PWM_RESOLUTION);
  ledcWrite(LED_PIN, 0);

  // Mercury switch
  pinMode(MERCURY_PIN, INPUT_PULLUP);

  // I2C + OLED
  Wire.begin(SDA_PIN, SCL_PIN);
  u8g2.begin();
  u8g2.setContrast(255);

  // RTC
  if (!rtc.begin(&Wire)) {
    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_6x10_tf);
    u8g2.drawStr(0, 10, "RTC NOT FOUND");
    u8g2.drawStr(0, 24, "Check wiring!");
    u8g2.sendBuffer();
    while (true) delay(1000);
  }
  statusRTC = true;
  if (rtc.lostPower()) rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));

  // Load settings from NVS
  loadPreferences();

  DateTime now = rtc.now();
  Serial.printf("[RTC] Time: %02d:%02d:%02d\n",
                now.hour(), now.minute(), now.second());

  // Determine initial LED cycle state
  int totalSeconds = now.hour() * 3600 + now.minute() * 60 + now.second();
  int offSeconds   = ledOffH * 3600 + ledOffM * 60;
  bool dayActive   = (activeDays >> now.dayOfTheWeek()) & 1;
  ledCycleComplete = (totalSeconds >= offSeconds) || !dayActive;
  ledcWrite(LED_PIN, 0);

  // Init mercury debounce and reset detection
  debouncedMercury = (digitalRead(MERCURY_PIN) == LOW);
  lastRawMercury   = debouncedMercury;
  resetLastState   = digitalRead(MERCURY_PIN);
  resetInitialized = true;
  resetWindowStart = millis();

  // Show time immediately
  drawDisplay(now, rtc.getTemperature());

  // Launch WiFi task on Core 0
  // (NTP + weather tasks are spawned inside wifiTask after connection)
  xTaskCreatePinnedToCore(wifiTask, "wifiTask", 8192, NULL, 1, NULL, 0);

  Serial.println("=== BOOT COMPLETE ===");
}

// ============================================================
void loop() {
  // Delayed restart (used after saving settings in AP mode)
  if (restartAt > 0 && millis() >= restartAt) ESP.restart();

  // Check for 3x ON reset sequence
  checkLiveReset();

  bool isOn = readMercury();
  DateTime now = rtc.now();

  // Mercury switch turned ON — reset LED cycle if within schedule
  if (isOn && !lastSwitchState) {
    int nowSeconds = now.hour() * 3600 + now.minute() * 60 + now.second();
    int offSeconds = ledOffH * 3600 + ledOffM * 60;
    bool dayActive = (activeDays >> now.dayOfTheWeek()) & 1;
    if (nowSeconds < offSeconds && dayActive) ledCycleComplete = false;
    ledcWrite(LED_PIN, 0);
    currentBrightness = 0;
  }
  lastSwitchState = isOn;

  // Switch OFF — blank display, keep SSE running
  if (!isOn) {
    ledcWrite(LED_PIN, 0);
    currentBrightness = 0;
    u8g2.clearBuffer();
    u8g2.sendBuffer();
    if (millis() - lastSSEUpdate >= SSE_INTERVAL) {
      lastSSEUpdate = millis();
      pushSSE();
    }
    delay(100);
    return;
  }

  // Switch ON — run LED schedule
  float temp = rtc.getTemperature();

  if (!ledCycleComplete) {
    bool dayActive = (activeDays >> now.dayOfTheWeek()) & 1;
    if (dayActive) {
      int brightness = computeLedBrightness(now.hour(), now.minute(), now.second());
      ledcWrite(LED_PIN, brightness);
      currentBrightness = brightness;
      if (brightness == 0 &&
          (now.hour() * 3600 + now.minute() * 60 + now.second()) >=
          (ledOffH * 3600 + ledOffM * 60)) {
        ledCycleComplete = true;
      }
    } else {
      ledcWrite(LED_PIN, 0);
      currentBrightness = 0;
    }
  }

  // SSE push
  if (millis() - lastSSEUpdate >= SSE_INTERVAL) {
    lastSSEUpdate = millis();
    pushSSE();
  }

  drawDisplay(now, temp);
  delay(100);
}
