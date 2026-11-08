#pragma once
#include <Arduino.h>

struct InputEvent {
  int8_t horizontal = 0;  // -1 previous, +1 next
  bool pressed = false;   // stable joystick button press
  bool activity = false;  // intentional press / movement
};

class InputController {
public:
  void begin();
  InputEvent poll(uint32_t now);
  void consumeWake();
private:
  bool armed_ = true;
  bool rawButton_ = false;
  bool stableButton_ = false;
  uint32_t rawChangedAt_ = 0;
};
