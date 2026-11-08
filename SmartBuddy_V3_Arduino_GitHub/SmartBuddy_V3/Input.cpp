#include "Input.h"
#include "Config.h"

void InputController::begin() {
  pinMode(SB::JOY_SW_PIN, INPUT_PULLUP);
  analogReadResolution(12);
  pinMode(SB::JOY_X_PIN, INPUT);
  pinMode(SB::JOY_Y_PIN, INPUT);
  rawButton_ = stableButton_ = (digitalRead(SB::JOY_SW_PIN) == LOW);
  rawChangedAt_ = millis();
}

InputEvent InputController::poll(uint32_t now) {
  InputEvent out;
  int x = analogRead(SB::JOY_X_PIN);
  bool centered = (x > SB::JOY_CENTER_LOW && x < SB::JOY_CENTER_HIGH);
  if (centered) armed_ = true;

  if (armed_ && (x < SB::JOY_LOW || x > SB::JOY_HIGH)) {
    out.horizontal = x > SB::JOY_HIGH ? +1 : -1;
    if (SB::INVERT_JOYSTICK_X) out.horizontal *= -1;
    out.activity = true;
    armed_ = false; // one page switch per movement, must recenter
  }

  bool raw = (digitalRead(SB::JOY_SW_PIN) == LOW);
  if (raw != rawButton_) {
    rawButton_ = raw;
    rawChangedAt_ = now;
  }
  if ((uint32_t)(now - rawChangedAt_) >= 45 && stableButton_ != rawButton_) {
    stableButton_ = rawButton_;
    if (stableButton_) {
      out.pressed = true;
      out.activity = true;
    }
  }
  return out;
}

void InputController::consumeWake() {
  armed_ = false; // prevent wake gesture from navigating another page
}
