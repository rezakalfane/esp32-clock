#pragma once

// FreeRTOS tasks — launched from wifiTask after connection
void wifiTask(void* param);
void ntpTask(void* param);
void weatherTask(void* param);

// Push current state to SSE clients
void pushSSE();
