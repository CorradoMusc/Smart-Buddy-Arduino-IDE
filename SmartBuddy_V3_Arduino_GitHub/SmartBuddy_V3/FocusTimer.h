#pragma once
#include <Arduino.h>
#include "Config.h"

class FocusTimer {
public:
  void press(uint32_t now);
  void update(uint32_t now);
  bool running() const { return running_; }
  bool complete() const { return remaining_ == 0 && !running_; }
  uint32_t remaining(uint32_t now) const;
  uint32_t elapsed(uint32_t now) const { return SB::FOCUS_DURATION_MS - remaining(now); }
  bool takeJustFinished();
private:
  bool running_ = false;
  bool justFinished_ = false;
  uint32_t remaining_ = SB::FOCUS_DURATION_MS;
  uint32_t deadline_ = 0;
};
