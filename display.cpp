#include "display.h"
#include "globals.h"
#include "config.h"
#include <U8g2lib.h>
#include <math.h>

extern U8G2_SSD1306_128X32_UNIVISION_F_HW_I2C u8g2;

// --- WiFi spinner frames 16x11 (arc build-up animation) ---
const unsigned char wspin0[] = {
  0x00,0x00, 0x00,0x00, 0x00,0x00, 0x00,0x00,
  0x00,0x00, 0x00,0x00, 0x00,0x00, 0x00,0x00,
  0x00,0x00, 0x01,0x80, 0x01,0x80
};
const unsigned char wspin1[] = {
  0x00,0x00, 0x00,0x00, 0x00,0x00, 0x00,0x00,
  0x00,0x00, 0x00,0x00, 0x01,0x80, 0x02,0x40,
  0x00,0x00, 0x01,0x80, 0x01,0x80
};
const unsigned char wspin2[] = {
  0x00,0x00, 0x00,0x00, 0x00,0x00, 0x03,0xC0,
  0x04,0x20, 0x08,0x10, 0x09,0x90, 0x02,0x40,
  0x00,0x00, 0x01,0x80, 0x01,0x80
};
const unsigned char wspin3[] = {
  0x07,0xE0, 0x08,0x10, 0x10,0x08, 0x23,0xC4,
  0x24,0x24, 0x08,0x10, 0x09,0x90, 0x02,0x40,
  0x00,0x00, 0x01,0x80, 0x01,0x80
};
const unsigned char* wifiSpinFrames[] = {
  wspin0, wspin1, wspin2, wspin3, wspin2, wspin1
};
#define WIFI_SPIN_FRAMES 6

// --- Round weather spinner 16x11 ---
const unsigned char rspin0[] = {
  0x03,0xC0, 0x0C,0x30, 0x10,0x08, 0x20,0x04,
  0x20,0x00, 0x20,0x00, 0x20,0x00, 0x20,0x04,
  0x10,0x08, 0x0C,0x30, 0x03,0xC0
};
const unsigned char rspin1[] = {
  0x03,0xC0, 0x0C,0x30, 0x10,0x08, 0x00,0x04,
  0x00,0x04, 0x00,0x04, 0x20,0x04, 0x20,0x04,
  0x10,0x08, 0x0C,0x30, 0x03,0xC0
};
const unsigned char rspin2[] = {
  0x03,0xC0, 0x0C,0x30, 0x10,0x08, 0x00,0x04,
  0x00,0x04, 0x00,0x04, 0x00,0x04, 0x20,0x04,
  0x30,0x08, 0x0C,0x30, 0x03,0xC0
};
const unsigned char rspin3[] = {
  0x03,0xC0, 0x0C,0x30, 0x30,0x08, 0x20,0x04,
  0x20,0x04, 0x20,0x04, 0x00,0x04, 0x00,0x04,
  0x10,0x08, 0x0C,0x30, 0x03,0xC0
};
const unsigned char* rspinFrames[] = { rspin0, rspin1, rspin2, rspin3 };
#define RSPIN_FRAMES 4

// --- Weather icons 16x11 ---
const unsigned char icon_sunny[] = {
  0x00,0x00, 0x08,0x10, 0xC1,0x83, 0x08,0x10,
  0x1C,0x38, 0x36,0x6C, 0x1C,0x38, 0x08,0x10,
  0xC1,0x83, 0x08,0x10, 0x00,0x00
};
const unsigned char icon_partly[] = {
  0x00,0x00, 0x38,0x00, 0x7C,0x00, 0xFE,0x00,
  0xFE,0x00, 0x0F,0xE0, 0x1F,0xF0, 0x3F,0xF8,
  0x3F,0xF8, 0x1F,0xF0, 0x00,0x00
};
const unsigned char icon_cloudy[] = {
  0x00,0x00, 0x00,0x00, 0x0F,0x80, 0x1F,0xC0,
  0x3F,0xE0, 0x7F,0xF0, 0xFF,0xF8, 0xFF,0xF8,
  0x7F,0xF0, 0x00,0x00, 0x00,0x00
};
const unsigned char icon_rainy[] = {
  0x00,0x00, 0x0F,0x80, 0x1F,0xC0, 0x7F,0xF0,
  0xFF,0xF8, 0xFF,0xF8, 0x7F,0xF0, 0x00,0x00,
  0x49,0x20, 0x92,0x40, 0x49,0x20
};
const unsigned char icon_snowy[] = {
  0x00,0x00, 0x0F,0x80, 0x1F,0xC0, 0x7F,0xF0,
  0xFF,0xF8, 0xFF,0xF8, 0x7F,0xF0, 0x00,0x00,
  0x55,0x50, 0x22,0x20, 0x55,0x50
};
const unsigned char icon_thunder[] = {
  0x00,0x00, 0x0F,0x80, 0x1F,0xC0, 0x7F,0xF0,
  0xFF,0xF8, 0xFF,0xF8, 0x7F,0xF0, 0x06,0x00,
  0x0F,0x00, 0x03,0x80, 0x01,0xC0
};

const unsigned char* getWeatherIcon(int code) {
  if (code == 0)                return icon_sunny;
  if (code >= 1 && code <= 2)   return icon_partly;
  if (code == 3)                return icon_cloudy;
  if (code >= 51 && code <= 82) return icon_rainy;
  if (code >= 71 && code <= 77) return icon_snowy;
  if (code >= 95 && code <= 99) return icon_thunder;
  return icon_cloudy;
}

// Draw WiFi icon/spinner left of temperature
static void drawWifiArea(int tempX) {
  int iconX = tempX - WIFI_ICON_W - 4;
  unsigned long ms = millis();

  if (wifiConnecting) {
    if (ms - lastWifiSpinUpdate > 200) {
      lastWifiSpinUpdate = ms;
      wifiSpinFrame = (wifiSpinFrame + 1) % WIFI_SPIN_FRAMES;
    }
    u8g2.drawBitmap(iconX, WIFI_ICON_Y, 2, WIFI_ICON_H, wifiSpinFrames[wifiSpinFrame]);
  } else if (statusWiFi) {
    u8g2.drawBitmap(iconX, WIFI_ICON_Y, 2, WIFI_ICON_H, wspin3);
  } else {
    u8g2.drawBitmap(iconX, WIFI_ICON_Y, 2, WIFI_ICON_H, wspin3);
    u8g2.drawLine(iconX, WIFI_ICON_Y,
                  iconX + WIFI_ICON_W - 1, WIFI_ICON_Y + WIFI_ICON_H - 1);
    u8g2.drawLine(iconX + WIFI_ICON_W - 1, WIFI_ICON_Y,
                  iconX, WIFI_ICON_Y + WIFI_ICON_H - 1);
  }
}

// Draw weather icon, weather spinner, or NTP T / client C indicator
static void drawBottomRightIcons() {
  unsigned long ms = millis();

  if (ntpSyncing && clientConnected) {
    // Alternate T and C while both are active
    if (ms - lastNtpBlink > 400) {
      lastNtpBlink  = ms;
      ntpBlinkState = !ntpBlinkState;
    }
    u8g2.setFont(u8g2_font_6x10_tf);
    u8g2.drawStr(NTP_ICON_X, 31, ntpBlinkState ? "T" : "C");
  } else if (ntpSyncing) {
    // Blink T only
    if (ms - lastNtpBlink > 400) {
      lastNtpBlink  = ms;
      ntpBlinkState = !ntpBlinkState;
    }
    if (ntpBlinkState) {
      u8g2.setFont(u8g2_font_6x10_tf);
      u8g2.drawStr(NTP_ICON_X, 31, "T");
    }
  } else if (clientConnected) {
    // Show C persistently while a client is connected
    u8g2.setFont(u8g2_font_6x10_tf);
    u8g2.drawStr(NTP_ICON_X, 31, "C");
  }

  // Weather spinner or icon
  if (weatherFetchInProgress) {
    if (ms - lastWeatherSpinUpdate > 150) {
      lastWeatherSpinUpdate = ms;
      weatherSpinFrame = (weatherSpinFrame + 1) % RSPIN_FRAMES;
    }
    u8g2.drawBitmap(ICON_X, ICON_Y, 2, 11, rspinFrames[weatherSpinFrame]);
  } else if (statusWeather) {
    u8g2.drawBitmap(ICON_X, ICON_Y, 2, 11, getWeatherIcon(weatherCode));
  }
}

void drawDisplay(DateTime now, float temp) {
  char timeBuf[9];
  snprintf(timeBuf, sizeof(timeBuf), "%02d:%02d:%02d",
           now.hour(), now.minute(), now.second());

  char dateBuf[16];
  snprintf(dateBuf, sizeof(dateBuf), "%s %02d/%02d/%04d",
           daysOfWeek[now.dayOfTheWeek()], now.day(), now.month(), now.year());

  char tempNum[6];
  snprintf(tempNum, sizeof(tempNum), "%d", (int)round(temp));

  u8g2.clearBuffer();

  // Time (large font)
  u8g2.setFont(u8g2_font_logisoso16_tf);
  u8g2.drawStr(0, 17, timeBuf);

  // Temperature top-right
  u8g2.setFont(u8g2_font_6x10_tf);
  int numWidth   = u8g2.getStrWidth(tempNum);
  int degWidth   = u8g2.getStrWidth("\xb0");
  int cWidth     = u8g2.getStrWidth("C");
  int totalWidth = numWidth + degWidth + cWidth;
  int tempX      = 128 - totalWidth;

  drawWifiArea(tempX);

  u8g2.drawStr(tempX, 12, tempNum);
  tempX += numWidth;
  u8g2.drawStr(tempX, 10, "\xb0");
  tempX += degWidth;
  u8g2.drawStr(tempX, 12, "C");

  // Divider
  u8g2.drawHLine(0, 20, 85);

  // Bottom row: feedback message or date + icons
  if (saveFeedback && millis() - saveFeedbackStart < 2000) {
    u8g2.setFont(u8g2_font_5x8_tf);
    u8g2.drawUTF8(0, 31, "Param\xc3\xa8tres enregistr\xc3\xa9s");
  } else {
    saveFeedback = false;
    u8g2.setFont(u8g2_font_6x10_tf);
    u8g2.drawStr(0, 31, dateBuf);
    drawBottomRightIcons();
  }

  u8g2.sendBuffer();
}
