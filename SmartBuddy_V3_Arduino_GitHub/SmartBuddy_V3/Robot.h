#pragma once
#include <TFT_eSPI.h>

enum class RobotMood : uint8_t { NEUTRAL, BLINK, HAPPY, FOCUS, SLEEPY, OFFLINE };

// Shapes are drawn procedurally. No bitmap assets or full-screen framebuffer needed.
class Robot {
public:
  static void draw(TFT_eSPI &g, int x, int y, int w, int h, RobotMood mood);
};
