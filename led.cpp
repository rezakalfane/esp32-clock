#include "led.h"
#include "globals.h"

int computeLedBrightness(int hour, int minute, int second) {
  int totalSeconds = hour * 3600 + minute * 60 + second;
  int startSeconds = rampStartH * 3600 + rampStartM * 60;
  int endSeconds   = rampEndH   * 3600 + rampEndM   * 60;
  int offSeconds   = ledOffH    * 3600 + ledOffM    * 60;

  if (totalSeconds < startSeconds) return 0;

  if (totalSeconds < endSeconds) {
    float progress = (float)(totalSeconds - startSeconds) /
                     (float)(endSeconds - startSeconds);
    return (int)(progress * 255.0);
  }

  if (totalSeconds < offSeconds) return 255;

  return 0;
}
