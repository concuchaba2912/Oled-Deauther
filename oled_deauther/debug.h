// Code by Nguyen The Hoang 
// Profile: https://facebook.com/boysitinh2912 https://github.com/concuchaba2912 https://www.youtube.com/watch?v=dQw4w9WgXcQ
// 90% chat bot AI 10% my code
#ifndef DEBUG_H
#define DEBUG_H

#include <Arduino.h>

//#define DEBUG
#define DEBUG_BAUD 115200

#ifdef DEBUG
#define DEBUG_SER_INIT() Serial.begin(DEBUG_BAUD);
#define DEBUG_SER_PRINT(...) Serial.print(__VA_ARGS__);
#else
#define DEBUG_SER_PRINT(...)
#define DEBUG_SER_INIT()
#endif

#endif
