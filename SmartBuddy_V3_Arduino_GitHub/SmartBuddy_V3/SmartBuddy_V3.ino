/*
 * SMART BUDDY V3 — ESP32-WROOM-32 + 4" ST7796S (480x320)
 * Four graphical pages + animated idle robot + reliable focus timer.
 *
 * SETUP FIRST:
 *   1. Copy Secrets.h.example to Secrets.h and fill in your Wi-Fi credentials.
 *   2. Keep your already working TFT_eSPI ST7796 configuration (pins in docs).
 *   3. Install TFT_eSPI, ArduinoJson 7, Adafruit DHT sensor library,
 *      and Adafruit Unified Sensor.
 *   4. Open THIS .ino file from the SmartBuddy_V3 folder and upload.
 */
#include <Arduino.h>
#include <WiFi.h>
#include <time.h>
#include "Secrets.h"
#include "Config.h"
#include "Model.h"
#include "UI.h"
#include "Input.h"
#include "Sensors.h"
#include "Weather.h"
#include "FocusTimer.h"

UI ui;
InputController input;
Sensors sensors;
WeatherService weather;
FocusTimer focus;

Page currentPage = Page::HOME;
WeatherReading currentWeather;
uint32_t lastInputMs = 0;
uint32_t lastWiFiCheckMs = 0;
uint32_t celebrateUntilMs = 0;
bool ntpStarted = false;

void startNetwork() {
  WiFi.mode(WIFI_STA);
  WiFi.setAutoReconnect(true);
  WiFi.begin(SB_WIFI_SSID, SB_WIFI_PASSWORD);
  Serial.println("[Wi-Fi] Connecting in background...");
}

void serviceWiFi(uint32_t now) {
  bool online = WiFi.status() == WL_CONNECTED;
  if (online && !ntpStarted) {
    configTzTime(SB::TIME_ZONE, "pool.ntp.org", "time.google.com");
    ntpStarted = true;
    Serial.println("[Clock] NTP synchronization requested");
  }
  if ((uint32_t)(now - lastWiFiCheckMs) < SB::WIFI_CHECK_MS) return;
  lastWiFiCheckMs = now;
  if (!online) {
    WiFi.reconnect();
  }
}

void setup() {
  Serial.begin(115200);
  delay(250);
  Serial.println("\n--- SMART BUDDY V3 ---");
  randomSeed(esp_random());
  input.begin();
  sensors.begin();
  ui.begin();
  startNetwork();
  weather.begin();
  lastInputMs = millis();
}

void loop() {
  uint32_t now = millis();
  serviceWiFi(now);
  sensors.poll(now);
  focus.update(now);

  if (focus.takeJustFinished()) {
    Serial.println("[Focus] Session complete!");
    celebrateUntilMs = now + 10000UL;
  }

  WeatherReading newWeather;
  if (weather.poll(newWeather)) currentWeather = newWeather;

  InputEvent ev = input.poll(now);
  bool wasIdle = (uint32_t)(now - lastInputMs) >= SB::IDLE_AFTER_MS;
  if (ev.activity) {
    lastInputMs = now;
    if (wasIdle) {
      // A wake action is consumed, so it cannot also change a page or timer.
      input.consumeWake();
    } else {
      if (ev.horizontal != 0) {
        int p = ((int)currentPage + (int)ev.horizontal + 4) % 4;
        currentPage = static_cast<Page>(p);
      }
      if (ev.pressed && currentPage == Page::FOCUS) {
        focus.press(now);
      }
    }
  }
  focus.update(now);

  AppView view;
  view.nowMs = now;
  view.page = currentPage;
  view.idleDurationMs = (uint32_t)(now - lastInputMs);
  view.idle = view.idleDurationMs >= SB::IDLE_AFTER_MS;
  view.wifiConnected = WiFi.status() == WL_CONNECTED;
  view.weather = currentWeather;
  view.room = sensors.snapshot();
  view.focusRunning = focus.running();
  view.focusComplete = focus.complete();
  view.focusRemainingMs = focus.remaining(now);
  view.focusElapsedMs = focus.elapsed(now);
  view.celebrate = celebrateUntilMs != 0 && (int32_t)(celebrateUntilMs - now) > 0;

  // NTP sets system time; localtime_r is nonblocking.
  time_t timestamp = time(nullptr);
  if (timestamp > 1700000000) {
    struct tm tmLocal;
    if (localtime_r(&timestamp, &tmLocal)) {
      view.clockValid = true;
      strftime(view.timeHHMM, sizeof(view.timeHHMM), "%I:%M", &tmLocal);
      strftime(view.timeSeconds, sizeof(view.timeSeconds), "%S", &tmLocal);
      strftime(view.dateLine, sizeof(view.dateLine), "%a, %b %d", &tmLocal);
    }
  }

  ui.render(view, now);
  delay(10);  // lets FreeRTOS/Wi-Fi tasks run; timer is NOT delay-based
}
