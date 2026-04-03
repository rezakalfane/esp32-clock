#include "mercury.h"
#include "globals.h"
#include "config.h"
#include "clockprefs.h"
#include <U8g2lib.h>

extern U8G2_SSD1306_128X32_UNIVISION_F_HW_I2C u8g2;

bool readMercury() {
  bool raw = (digitalRead(MERCURY_PIN) == LOW);
  unsigned long now = millis();

  // Reset timer on every raw change — only commit when signal has been stable for full debounce period
  if (raw != lastRawMercury) {
    lastRawMercury      = raw;
    lastMercuryDebounce = now;
  } else if (now - lastMercuryDebounce >= MERCURY_DEBOUNCE_MS) {
    debouncedMercury = raw;
  }

  return debouncedMercury;
}

void checkLiveReset() {
  if (!resetInitialized) {
    resetLastState   = digitalRead(MERCURY_PIN);
    resetInitialized = true;
    resetWindowStart = millis();
  }

  bool currentState = digitalRead(MERCURY_PIN);

  if (!resetPendingChange && currentState != resetLastState) {
    resetPendingChange = true;
    resetPendingState  = currentState;
    resetDebounceStart = millis();
  }

  if (resetPendingChange) {
    if (digitalRead(MERCURY_PIN) != resetPendingState) {
      resetPendingChange = false;
    } else if (millis() - resetDebounceStart >= RESET_DEBOUNCE_MS) {
      resetLastState     = resetPendingState;
      resetPendingChange = false;
      resetToggleCount++;

      if (resetLastState == LOW) {
        resetOnCount++;
        Serial.print("[Reset] ON pulse #"); Serial.println(resetOnCount);
      }

      Serial.print("[Reset] Toggle "); Serial.print(resetToggleCount);
      Serial.println(resetLastState == LOW ? " -> ON" : " -> OFF");

      if (resetToggleCount == 1) {
        resetWindowStart = millis();
        resetOnCount     = (resetLastState == LOW) ? 1 : 0;
      }

      // Trigger on 3 ON pulses within window
      if (resetOnCount >= RESET_ON_TARGET &&
          millis() - resetWindowStart <= RESET_WINDOW_MS) {
        Serial.println("[Reset] 3 ON pulses — resetting settings!");

        u8g2.clearBuffer();
        u8g2.setFont(u8g2_font_6x10_tf);
        u8g2.drawStr(0, 12, "Reinitialisation");
        u8g2.drawStr(0, 26, "des parametres...");
        u8g2.sendBuffer();
        resetToDefaults();
        delay(1000);

        u8g2.clearBuffer();
        u8g2.setFont(u8g2_font_6x10_tf);
        u8g2.drawStr(0, 12, "Redemarrage...");
        u8g2.sendBuffer();
        delay(1000);

        ESP.restart();
      }

      // Window expired — reset counters
      if (millis() - resetWindowStart > RESET_WINDOW_MS) {
        Serial.println("[Reset] Window expired, restarting count");
        resetToggleCount = 0;
        resetOnCount     = 0;
        resetWindowStart = millis();
      }
    }
  }
}
