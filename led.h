#pragma once

// Returns PWM brightness 0-255 based on current time and schedule
int computeLedBrightness(int hour, int minute, int second);
