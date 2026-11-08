#pragma once
#include <Arduino.h>

namespace SB {
// Your working ESP32-WROOM-32 / ST7796S hardware pin assignment.
constexpr uint8_t DHT_PIN = 4;
constexpr uint8_t JOY_X_PIN = 34;
constexpr uint8_t JOY_Y_PIN = 35;
constexpr uint8_t JOY_SW_PIN = 32;

constexpr uint32_t SENSOR_PERIOD_MS = 2500;
constexpr uint32_t WIFI_CHECK_MS = 10000;
constexpr uint32_t WEATHER_PERIOD_MS = 15UL * 60UL * 1000UL;
constexpr uint32_t WEATHER_RETRY_MS = 60UL * 1000UL;
constexpr uint32_t IDLE_AFTER_MS = 60UL * 1000UL;
constexpr uint32_t SLEEPY_AFTER_MS = 5UL * 60UL * 1000UL;
constexpr uint32_t FOCUS_DURATION_MS = 25UL * 60UL * 1000UL;

constexpr int JOY_LOW = 850;
constexpr int JOY_HIGH = 3250;
constexpr int JOY_CENTER_LOW = 1400;
constexpr int JOY_CENTER_HIGH = 2700;
constexpr bool INVERT_JOYSTICK_X = false;

// Markham, ON example: change these to your desired weather location.
constexpr float WEATHER_LATITUDE = 43.8561f;
constexpr float WEATHER_LONGITUDE = -79.3370f;
constexpr char LOCATION_NAME[] = "Markham";
constexpr char TIME_ZONE[] = "EST5EDT,M3.2.0/2,M11.1.0/2";

constexpr uint16_t rgb565(uint8_t r, uint8_t g, uint8_t b) {
  return ((uint16_t)(r & 0xF8) << 8) | ((uint16_t)(g & 0xFC) << 3) | (b >> 3);
}
constexpr uint16_t BG      = rgb565(0x0B, 0x10, 0x20);
constexpr uint16_t CARD    = rgb565(0x16, 0x24, 0x3A);
constexpr uint16_t EDGE    = rgb565(0x2C, 0x40, 0x58);
constexpr uint16_t WHITE   = rgb565(0xF3, 0xF7, 0xFC);
constexpr uint16_t MUTED   = rgb565(0x9B, 0xB0, 0xC6);
constexpr uint16_t CYAN    = rgb565(0x51, 0xD6, 0xFF);
constexpr uint16_t MINT    = rgb565(0x7C, 0xE5, 0xB6);
constexpr uint16_t AMBER   = rgb565(0xFF, 0xC9, 0x68);
constexpr uint16_t RED     = rgb565(0xFB, 0x7C, 0x88);
constexpr uint16_t ROBOT   = rgb565(0x28, 0x42, 0x65);
}
