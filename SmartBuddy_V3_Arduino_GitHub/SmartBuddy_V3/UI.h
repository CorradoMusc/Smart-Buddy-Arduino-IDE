#pragma once
#include <TFT_eSPI.h>
#include "Model.h"
#include "Robot.h"

class UI {
public:
  UI();
  void begin();
  void render(const AppView &state, uint32_t now);
private:
  TFT_eSPI tft_;
  TFT_eSprite clockSprite_;
  TFT_eSprite cardSprite_;
  TFT_eSprite robotSprite_;
  bool clockSpriteOk_ = false;
  bool cardSpriteOk_ = false;
  bool robotSpriteOk_ = false;
  Page drawnPage_ = Page::HOME;
  bool drawnIdle_ = false;
  bool initialized_ = false;
  bool lastWifi_ = false;
  uint32_t lastUpdate_ = 0;
  uint32_t lastIdleFrame_ = 0;
  uint32_t blinkAt_ = 0;
  uint32_t blinkEnd_ = 0;
  bool transitionActive_ = false;
  uint32_t transitionStartMs_ = 0;

  void drawBase(const AppView &state);
  void drawHeader(const AppView &state);
  void drawFooter(const AppView &state);
  void drawWifi(bool connected);
  void drawCard(int x, int y, const char *title, const String &value,
                const String &caption, uint8_t icon, uint16_t accent);
  void drawClockPanel(const AppView &state, int x, int y, bool centered);
  void drawPageValues(const AppView &state);
  void drawIdle(const AppView &state, uint32_t now);
};
