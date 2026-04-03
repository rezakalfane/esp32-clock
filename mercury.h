#pragma once

// Returns debounced mercury switch state (true = ON/closed)
bool readMercury();

// Call every loop() iteration — detects 3x ON pattern and triggers reset
void checkLiveReset();
