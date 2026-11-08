#include "FocusTimer.h"

uint32_t FocusTimer::remaining(uint32_t now) const {
  if (!running_) return remaining_;
  int32_t delta = (int32_t)(deadline_ - now);
  return delta > 0 ? (uint32_t)delta : 0;
}

void FocusTimer::update(uint32_t now) {
  if (running_ && remaining(now) == 0) {
    running_ = false;
    remaining_ = 0;
    justFinished_ = true;
  }
}

void FocusTimer::press(uint32_t now) {
  update(now);
  if (running_) {
    remaining_ = remaining(now);
    running_ = false;
  } else {
    if (remaining_ == 0) {
      remaining_ = SB::FOCUS_DURATION_MS;
      justFinished_ = false;
    }
    deadline_ = now + remaining_;
    running_ = true;
  }
}

bool FocusTimer::takeJustFinished() {
  bool event = justFinished_;
  justFinished_ = false;
  return event;
}
