#pragma once
#include <RTClib.h>

// Draw the full OLED display frame
void drawDisplay(DateTime now, float temp);

// Trigger a sliding notification on the bottom row
void triggerNotif(const char* text, unsigned long durationMs, bool smallFont = false, bool isUTF8 = false);
