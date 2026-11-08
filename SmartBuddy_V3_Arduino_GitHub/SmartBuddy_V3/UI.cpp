#include "UI.h"
#include "Config.h"
#include <WiFi.h>

namespace {
// Compact vector weather illustrations. Works on TFT_eSPI and its sprites.
void sun(TFT_eSPI &g, int x, int y, int radius) {
  g.fillCircle(x, y, radius, SB::AMBER);
  for (int i = 0; i < 8; ++i) {
    // Deliberately avoid float/sin: fixed integer spokes.
    static const int dx[8] = {0, 1, 1, 1, 0, -1, -1, -1};
    static const int dy[8] = {-1, -1, 0, 1, 1, 1, 0, -1};
    g.drawLine(x + dx[i] * (radius + 3), y + dy[i] * (radius + 3),
               x + dx[i] * (radius + 8), y + dy[i] * (radius + 8), SB::AMBER);
  }
}
void cloud(TFT_eSPI &g, int x, int y, int sz) {
  g.fillCircle(x - sz / 5, y, sz / 3, SB::WHITE);
  g.fillCircle(x + sz / 7, y - sz / 7, sz / 2, SB::WHITE);
  g.fillCircle(x + sz / 2, y + sz / 10, sz / 3, SB::WHITE);
  g.fillRoundRect(x - sz / 2, y, sz + sz / 3, sz / 2, sz / 8, SB::WHITE);
}
void weatherIcon(TFT_eSPI &g, int code, int x, int y, int size) {
  if (code == 0) { sun(g, x, y, size / 3); return; }
  if (code >= 1 && code <= 3) {
    sun(g, x - size / 4, y - size / 4, size / 5);
    cloud(g, x + size / 5, y + size / 6, size / 2);
    return;
  }
  cloud(g, x, y - size / 8, size / 2);
  if ((code >= 51 && code <= 67) || (code >= 80 && code <= 82) || code >= 95) {
    for (int i = -1; i <= 1; ++i)
      g.drawLine(x + i * 15, y + size / 3, x + i * 15 - 4, y + size / 2, SB::CYAN);
  } else if ((code >= 71 && code <= 77) || (code >= 85 && code <= 86)) {
    for (int i = -1; i <= 1; ++i) g.fillCircle(x + i * 15, y + size / 3, 3, SB::CYAN);
  }
}
const char* description(int code) {
  if (code == 0) return "Clear sky";
  if (code == 1) return "Mostly clear";
  if (code == 2) return "Partly cloudy";
  if (code == 3) return "Overcast";
  if (code == 45 || code == 48) return "Foggy";
  if (code >= 51 && code <= 57) return "Drizzle";
  if (code >= 61 && code <= 67) return "Rain";
  if (code >= 71 && code <= 77) return "Snow";
  if (code >= 80 && code <= 82) return "Showers";
  if (code >= 85 && code <= 86) return "Snow showers";
  if (code >= 95) return "Thunderstorm";
  return "Conditions unknown";
}
const char* comfort(const RoomReading &r) {
  if (!r.valid) return "No sensor data";
  if (r.humidity < 30) return "Dry air";
  if (r.humidity > 60) return "Slightly humid";
  return "Comfort range";
}
String temperature(float value) {
  return isnan(value) ? String("--.- C") : String(value, 1) + " C";
}
}

UI::UI() : tft_(), clockSprite_(&tft_), cardSprite_(&tft_), robotSprite_(&tft_) {}

void UI::begin() {
  tft_.init();
  tft_.setRotation(1);  // 480 x 320 landscape
  tft_.fillScreen(SB::BG);
  clockSprite_.setColorDepth(8);
  cardSprite_.setColorDepth(8);
  robotSprite_.setColorDepth(8);
  clockSpriteOk_ = clockSprite_.createSprite(300, 105) != nullptr;
  cardSpriteOk_ = cardSprite_.createSprite(216, 96) != nullptr;
  robotSpriteOk_ = robotSprite_.createSprite(192, 164) != nullptr;
  Serial.printf("[UI] Sprites: clock=%d card=%d robot=%d, free heap=%u\n",
    clockSpriteOk_, cardSpriteOk_, robotSpriteOk_, (unsigned)ESP.getFreeHeap());
  if (!clockSpriteOk_ || !cardSpriteOk_ || !robotSpriteOk_) {
    Serial.println("[UI] Sprite allocation failed; smaller direct fallback regions will be used.");
  }
  blinkAt_ = millis() + 5000;
  initialized_ = false;
}

void UI::drawWifi(bool connected) {
  tft_.fillRoundRect(377, 8, 88, 24, 10, SB::CARD);
  tft_.fillCircle(389, 20, 4, connected ? SB::MINT : SB::RED);
  tft_.setTextColor(connected ? SB::MINT : SB::RED, SB::CARD);
  tft_.drawString(connected ? "ONLINE" : "OFFLINE", 399, 14, 2);
}

void UI::drawHeader(const AppView &state) {
  tft_.fillRect(0, 0, 480, 38, SB::BG);
  tft_.setTextDatum(TL_DATUM);
  tft_.setTextColor(SB::CYAN, SB::BG);
  tft_.drawString("SMART BUDDY", 17, 10, 2);
  drawWifi(state.wifiConnected);
  tft_.drawFastHLine(17, 37, 446, SB::EDGE);
}

void UI::drawFooter(const AppView &state) {
  tft_.fillRect(0, 286, 480, 34, SB::BG);
  tft_.drawFastHLine(17, 286, 446, SB::EDGE);
  tft_.setTextColor(SB::MUTED, SB::BG);
  if (state.idle) {
    tft_.drawCentreString("MOVE OR PRESS TO WAKE", 240, 296, 2);
    return;
  }
  tft_.drawString("< PREV", 20, 295, 2);
  tft_.drawRightString("NEXT >", 460, 295, 2);
  for (int i = 0; i < 4; ++i)
    tft_.fillCircle(221 + i * 13, 304, i == (int)state.page ? 4 : 3,
                    i == (int)state.page ? SB::CYAN : SB::EDGE);
}

void UI::drawBase(const AppView &state) {
  tft_.fillScreen(SB::BG);  // only on PAGE CHANGE / idle entry and exit
  drawHeader(state);
  drawFooter(state);
  if (state.idle) return;
  const char* titles[] = {"HOME", "WEATHER", "MY ROOM", "FOCUS TIMER"};
  tft_.setTextDatum(TL_DATUM);
  tft_.setTextColor(SB::AMBER, SB::BG);
  tft_.drawString(titles[(int)state.page], 18, 45, 2);
  if (state.page == Page::HOME) {
    Robot::draw(tft_, 352, 73, 96, 99, RobotMood::NEUTRAL);
  } else if (state.page == Page::FOCUS) {
    Robot::draw(tft_, 368, 91, 80, 92, RobotMood::FOCUS);
  }
}

void UI::drawCard(int x, int y, const char *title, const String &value,
                  const String &caption, uint8_t icon, uint16_t accent) {
  TFT_eSPI *g = cardSpriteOk_ ? static_cast<TFT_eSPI*>(&cardSprite_) : &tft_;
  int ox = cardSpriteOk_ ? 0 : x;
  int oy = cardSpriteOk_ ? 0 : y;
  if (cardSpriteOk_) cardSprite_.fillSprite(SB::BG);
  g->fillRoundRect(ox, oy, 214, 95, 12, SB::CARD);
  g->drawRoundRect(ox, oy, 214, 95, 12, SB::EDGE);
  g->setTextDatum(TL_DATUM);
  g->setTextColor(SB::MUTED, SB::CARD);
  g->drawString(title, ox + 13, oy + 11, 2);
  g->setTextColor(SB::WHITE, SB::CARD);
  g->drawString(value, ox + 12, oy + 37, 4);
  g->setTextColor(SB::MUTED, SB::CARD);
  g->drawString(caption, ox + 13, oy + 75, 2);
  if (icon == 1) weatherIcon(*g, 2, ox + 180, oy + 34, 28);
  else if (icon == 2) {
    g->drawRoundRect(ox + 174, oy + 22, 12, 30, 5, accent);
    g->fillCircle(ox + 180, oy + 50, 8, accent);
  } else if (icon == 3) {
    g->fillCircle(ox + 180, oy + 40, 8, accent);
    g->fillTriangle(ox + 180, oy + 22, ox + 174, oy + 34, ox + 186, oy + 34, accent);
  }
  if (cardSpriteOk_) cardSprite_.pushSprite(x, y);
}

void UI::drawClockPanel(const AppView &state, int x, int y, bool centered) {
  TFT_eSPI *g = clockSpriteOk_ ? static_cast<TFT_eSPI*>(&clockSprite_) : &tft_;
  int ox = clockSpriteOk_ ? 0 : x;
  int oy = clockSpriteOk_ ? 0 : y;
  if (clockSpriteOk_) clockSprite_.fillSprite(SB::BG);
  g->setTextDatum(TL_DATUM);
  g->setTextColor(SB::WHITE, SB::BG);
  const char* v = state.clockValid ? state.timeHHMM : "--:--";
  if (centered) {
    g->drawCentreString(v, ox + 150, oy + 6, 7);
  } else {
    g->drawString(v, ox + 2, oy + 3, 7);
  }
  g->setTextColor(SB::MUTED, SB::BG);
  g->drawString(state.dateLine, ox + (centered ? 40 : 4), oy + 78, 2);
  if (clockSpriteOk_) clockSprite_.pushSprite(x, y);
}

void UI::drawPageValues(const AppView &state) {
  const auto &w = state.weather;
  const auto &r = state.room;
  if (state.page == Page::HOME) {
    drawClockPanel(state, 18, 69, false);
    String outsideStatus = !w.valid ? "Waiting for Wi-Fi" :
      ((uint32_t)(state.nowMs - w.updatedMs) > 45UL * 60000UL ? "Weather data stale" : String(description(w.code)));
    String roomStatus = !r.valid ? "Sensor unavailable" :
      ((uint32_t)(state.nowMs - r.updatedMs) > 30000UL ? "Sensor data stale" : String(r.humidity, 0) + "% humidity");
    drawCard(18, 184, "OUTDOORS", w.valid ? temperature(w.temperature) : "--.- C",
             outsideStatus, 1, SB::AMBER);
    drawCard(247, 184, "MY ROOM", r.valid ? temperature(r.temperature) : "--.- C",
             roomStatus, 2, SB::MINT);
    return;
  }
  if (state.page == Page::WEATHER) {
    tft_.fillRoundRect(22, 77, 124, 101, 14, SB::CARD);
    weatherIcon(tft_, w.valid ? w.code : 2, 73, 119, 77);
    // Composited 8-bit sprite prevents visible erase/redraw flashing.
    if (clockSpriteOk_) {
      clockSprite_.fillSprite(SB::BG);
      clockSprite_.setTextDatum(TL_DATUM);
      clockSprite_.setTextColor(SB::WHITE, SB::BG);
      clockSprite_.drawString(w.valid ? String(w.temperature, 1) : "--.-", 0, 5, 7);
      clockSprite_.setTextColor(SB::CYAN, SB::BG);
      clockSprite_.drawString("C", 237, 38, 4);
      clockSprite_.setTextColor(SB::MUTED, SB::BG);
      clockSprite_.drawString(w.valid ? description(w.code) : "Weather unavailable", 2, 75, 2);
      clockSprite_.pushSprite(158, 77);
    } else {
      tft_.fillRect(155, 79, 308, 105, SB::BG);
      tft_.setTextColor(SB::WHITE, SB::BG);
      tft_.drawString(w.valid ? temperature(w.temperature) : "--.- C", 162, 90, 4);
      tft_.setTextColor(SB::MUTED, SB::BG);
      tft_.drawString(w.valid ? description(w.code) : "Weather unavailable", 162, 150, 2);
    }
    drawCard(18, 184, "FEELS LIKE", w.valid ? temperature(w.apparent) : "--.- C",
             SB::LOCATION_NAME, 2, SB::CYAN);
    drawCard(247, 184, "OUTDOOR HUMIDITY", w.valid ? String(w.humidity) + "%" : "--%",
             w.valid ? String("Wind ") + String(w.windKmh, 1) + " km/h" : "No weather yet", 3, SB::CYAN);
    // Last success age: show stale data explicitly, never pretend it is live.
    tft_.fillRect(290, 47, 171, 19, SB::BG);
    tft_.setTextColor(SB::MUTED, SB::BG);
    if (w.valid) {
      uint32_t age = (uint32_t)(state.nowMs - w.updatedMs) / 60000UL;
      tft_.setTextColor(age > 45 ? SB::AMBER : SB::MUTED, SB::BG);
      tft_.drawRightString(age > 45 ? "WEATHER STALE" :
                           String("Updated ") + String(age) + "m ago", 460, 47, 2);
    } else {
      tft_.drawRightString("Waiting for data", 460, 47, 2);
    }
    return;
  }
  if (state.page == Page::ROOM) {
    if (clockSpriteOk_) {
      clockSprite_.fillSprite(SB::BG);
      clockSprite_.setTextDatum(TL_DATUM);
      clockSprite_.setTextColor(SB::WHITE, SB::BG);
      clockSprite_.drawString(r.valid ? String(r.temperature, 1) : "--.-", 0, 2, 7);
      clockSprite_.setTextColor(SB::CYAN, SB::BG);
      clockSprite_.drawString("C", 237, 35, 4);
      clockSprite_.setTextColor(SB::MUTED, SB::BG);
      clockSprite_.drawString("Room temperature", 5, 77, 2);
      clockSprite_.pushSprite(18, 79);
    } else {
      tft_.fillRect(16, 82, 448, 98, SB::BG);
      tft_.setTextColor(SB::WHITE, SB::BG);
      tft_.drawString(r.valid ? temperature(r.temperature) : "--.- C", 22, 97, 4);
    }
    // Old DHT11 readings are labelled if the sensor stops responding.
    if (r.valid && (uint32_t)(state.nowMs - r.updatedMs) > 30000UL) {
      tft_.setTextColor(SB::AMBER, SB::BG);
      tft_.drawString("Sensor data stale", 320, 141, 2);
    } else {
      tft_.fillRect(320, 139, 143, 25, SB::BG);
    }
    drawCard(18, 184, "RELATIVE HUMIDITY", r.valid ? String(r.humidity, 0) + "%" : "--%",
             "DHT11 sensor", 3, SB::CYAN);
    drawCard(247, 184, "COMFORT ESTIMATE", r.valid ? String(comfort(r)) : "Unavailable",
             "Humidity based", 2, SB::MINT);
    return;
  }
  if (state.page == Page::FOCUS) {
    uint32_t seconds = (state.focusRemainingMs + 999UL) / 1000UL;
    char timer[16];
    snprintf(timer, sizeof(timer), "%02lu:%02lu", (unsigned long)(seconds / 60), (unsigned long)(seconds % 60));
    if (clockSpriteOk_) {
      clockSprite_.fillSprite(SB::BG);
      clockSprite_.setTextDatum(TL_DATUM);
      clockSprite_.setTextColor(SB::CYAN, SB::BG);
      clockSprite_.drawCentreString(timer, 150, 14, 7);
      clockSprite_.pushSprite(78, 88);
    } else {
      tft_.fillRect(72, 88, 294, 93, SB::BG);
      tft_.setTextColor(SB::CYAN, SB::BG);
      tft_.drawString(timer, 93, 99, 7);
    }
    tft_.fillRoundRect(32, 204, 416, 14, 7, SB::EDGE);
    uint32_t filled = (uint32_t)((uint64_t)state.focusElapsedMs * 408UL / SB::FOCUS_DURATION_MS);
    if (filled > 408) filled = 408;
    if (filled > 0) tft_.fillRoundRect(36, 208, filled, 6, 3, SB::CYAN);
    tft_.fillRect(18, 230, 444, 44, SB::BG);
    tft_.setTextColor(state.focusComplete ? SB::MINT : SB::MUTED, SB::BG);
    const char* status = state.focusComplete ? "SESSION COMPLETE!" :
                         state.focusRunning ? "FOCUSING" :
                         (state.focusElapsedMs == 0 ? "READY TO START" : "PAUSED");
    tft_.drawCentreString(status, 240, 232, 4);
    tft_.setTextColor(SB::MUTED, SB::BG);
    tft_.drawCentreString("PRESS JOYSTICK TO START / PAUSE", 240, 267, 2);
  }
}

void UI::drawIdle(const AppView &state, uint32_t now) {
  int offset = (now / 280) % 4;
  int bob = (offset == 0 ? 0 : offset == 1 ? 1 : offset == 2 ? 2 : 1);
  if ((int32_t)(now - blinkAt_) >= 0 && blinkEnd_ == 0) {
    blinkEnd_ = now + 150;
  }
  bool blinking = blinkEnd_ != 0 && (int32_t)(blinkEnd_ - now) > 0;
  if (blinkEnd_ != 0 && !blinking) {
    blinkEnd_ = 0;
    blinkAt_ = now + 4000UL + random(0, 4000);
  }
  RobotMood mood = state.celebrate ? RobotMood::HAPPY :
                   blinking ? RobotMood::BLINK :
                   state.focusRunning ? RobotMood::FOCUS :
                   state.idleDurationMs >= SB::SLEEPY_AFTER_MS ? RobotMood::SLEEPY :
                   !state.wifiConnected ? RobotMood::OFFLINE : RobotMood::NEUTRAL;
  if (robotSpriteOk_) {
    robotSprite_.fillSprite(SB::BG);
    Robot::draw(robotSprite_, 8, bob, 176, 156, mood);
    robotSprite_.pushSprite(144, 73);
  } else {
    tft_.fillRect(144, 73, 192, 164, SB::BG);
    Robot::draw(tft_, 150, 73 + bob, 176, 156, mood);
  }
  tft_.fillRect(35, 240, 410, 40, SB::BG);
  tft_.setTextColor(SB::WHITE, SB::BG);
  const char* phrase = state.celebrate ? "Nice work! Session complete!" :
                       state.focusRunning ? "Focus time. You got this!" :
                       !state.wifiConnected ? "Waiting for Wi-Fi..." :
                       state.idleDurationMs >= SB::SLEEPY_AFTER_MS ? "Zzz..." :
                       "All systems looking good!";
  tft_.drawCentreString(phrase, 240, 241, 2);
  if (state.focusRunning) {
    uint32_t sec = (state.focusRemainingMs + 999) / 1000;
    char remaining[20];
    snprintf(remaining, sizeof(remaining), "Focus %02lu:%02lu", (unsigned long)(sec / 60), (unsigned long)(sec % 60));
    tft_.setTextColor(SB::CYAN, SB::BG);
    tft_.drawCentreString(remaining, 240, 262, 2);
  } else {
    tft_.setTextColor(SB::MUTED, SB::BG);
    tft_.drawCentreString(state.timeHHMM, 240, 262, 2);
  }
}

void UI::render(const AppView &state, uint32_t now) {
  bool pageChanged = !initialized_ || drawnIdle_ != state.idle ||
                     (!state.idle && drawnPage_ != state.page);
  if (pageChanged) {
    drawnIdle_ = state.idle;
    drawnPage_ = state.page;
    drawBase(state);
    lastUpdate_ = 0;
    lastIdleFrame_ = 0;
    initialized_ = true;
    lastWifi_ = state.wifiConnected;
    // A subtle nonblocking accent sweep on page changes (no full-screen buffers).
    transitionActive_ = !state.idle;
    transitionStartMs_ = now;
  }
  if (lastWifi_ != state.wifiConnected) {
    drawWifi(state.wifiConnected);
    lastWifi_ = state.wifiConnected;
  }
  if (state.idle) {
    if (pageChanged || (uint32_t)(now - lastIdleFrame_) >= 220) {
      lastIdleFrame_ = now;
      drawIdle(state, now);
    }
  } else {
    const uint32_t interval = (state.page == Page::HOME || state.page == Page::FOCUS) ? 1000 : 2500;
    if (pageChanged || (uint32_t)(now - lastUpdate_) >= interval) {
      lastUpdate_ = now;
      drawPageValues(state);
    }
  }
  if (transitionActive_) {
    uint32_t elapsed = (uint32_t)(now - transitionStartMs_);
    tft_.fillRect(17, 39, 446, 3, SB::EDGE);
    if (elapsed < 200) {
      uint32_t w = (elapsed * 446UL) / 200UL;
      if (w > 0) tft_.fillRect(17, 39, w, 3, SB::CYAN);
    } else {
      transitionActive_ = false;
    }
  }
}
