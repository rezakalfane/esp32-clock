#pragma once
#include <Arduino.h>
#include <RTClib.h>

// Forward declarations instead of full includes
class AsyncWebServer;
class AsyncEventSource;

// --- Hardware ---
extern RTC_DS3231 rtc;
extern AsyncWebServer server;
extern AsyncEventSource events;

// --- Settings (persisted in NVS) ---
extern String  wifiSSID;
extern String  wifiPassword;
extern String  latitude;
extern String  longitude;
extern int     rampStartH;
extern int     rampStartM;
extern int     rampEndH;
extern int     rampEndM;
extern int     ledOffH;
extern int     ledOffM;
extern int     activeDays;

// --- Runtime state ---
extern bool ledCycleComplete;
extern bool lastSwitchState;
extern bool statusRTC;
extern bool statusWiFi;
extern bool statusWeather;
extern bool weatherFetchFailed;
extern int  weatherCode;
extern int  currentBrightness;

// --- Display animation ---
extern int  wifiSpinFrame;
extern int  weatherSpinFrame;
extern unsigned long lastWifiSpinUpdate;
extern unsigned long lastWeatherSpinUpdate;
extern unsigned long lastNtpBlink;
extern bool ntpBlinkState;
extern bool          clientConnected;
extern bool          apMode;
extern unsigned long restartAt;
// --- Sliding notification ---
enum NotifState { NOTIF_IDLE, NOTIF_ENTERING, NOTIF_VISIBLE, NOTIF_LEAVING };
extern NotifState    notifState;
extern int           notifSlideY;
extern unsigned long notifLastStep;
extern unsigned long notifShowUntil;
extern unsigned long notifDuration;
extern char          notifText[32];
extern bool          notifSmallFont;
extern bool          notifIsUTF8;

// --- Timing ---
extern unsigned long lastWeatherFetch;
extern unsigned long lastSSEUpdate;

// --- Task flags ---
extern volatile bool wifiConnecting;
extern volatile bool weatherFetchInProgress;
extern volatile bool ntpSyncing;

// --- Mercury debounce ---
extern unsigned long lastMercuryDebounce;
extern bool          debouncedMercury;
extern bool          lastRawMercury;

// --- Live reset detection ---
extern int           resetToggleCount;
extern int           resetOnCount;
extern unsigned long resetWindowStart;
extern unsigned long resetDebounceStart;
extern bool          resetPendingChange;
extern bool          resetPendingState;
extern bool          resetLastState;
extern bool          resetInitialized;

// --- Day names ---
extern const char* daysOfWeek[];
