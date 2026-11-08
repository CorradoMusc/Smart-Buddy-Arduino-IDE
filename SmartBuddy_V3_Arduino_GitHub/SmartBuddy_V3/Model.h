#pragma once
#include <Arduino.h>
#include <math.h>

enum class Page : uint8_t { HOME = 0, WEATHER = 1, ROOM = 2, FOCUS = 3 };

struct WeatherReading {
  bool valid = false;
  float temperature = NAN;
  float apparent = NAN;
  float windKmh = NAN;
  int humidity = -1;
  int code = -1;
  uint32_t updatedMs = 0;
};

struct RoomReading {
  bool valid = false;
  float temperature = NAN;
  float humidity = NAN;
  uint32_t updatedMs = 0;
};

struct AppView {
  Page page = Page::HOME;
  bool idle = false;
  bool wifiConnected = false;
  bool clockValid = false;
  char timeHHMM[12] = "--:--";
  char timeSeconds[8] = "--";
  char dateLine[40] = "Syncing time...";
  WeatherReading weather;
  RoomReading room;
  bool focusRunning = false;
  bool focusComplete = false;
  uint32_t focusRemainingMs = 25UL * 60UL * 1000UL;
  uint32_t focusElapsedMs = 0;
  uint32_t idleDurationMs = 0;
  bool celebrate = false;
  uint32_t nowMs = 0;
};
