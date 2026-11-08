#include "Robot.h"
#include "Config.h"

void Robot::draw(TFT_eSPI &g, int x, int y, int w, int h, RobotMood mood) {
  const int cx = x + w / 2;
  const int antennaY = y + 5;
  const int bodyX = x + w / 10;
  const int bodyY = y + h / 7;
  const int bodyW = w - 2 * (w / 10);
  const int bodyH = h * 72 / 100;
  const int radius = (w < 110) ? 14 : 27;
  const int faceX = bodyX + bodyW / 10;
  const int faceY = bodyY + bodyH / 5;
  const int faceW = bodyW - 2 * (bodyW / 10);
  const int faceH = bodyH * 60 / 100;
  const int eyeY = faceY + faceH * 45 / 100;
  const int eyeDX = faceW / 5;
  const int eyeW = (w < 110) ? 7 : 14;
  const int eyeH = (w < 110) ? 13 : 24;

  g.drawLine(cx, antennaY + 10, cx, bodyY, SB::CYAN);
  g.fillCircle(cx, antennaY + 5, (w < 110 ? 3 : 6), SB::AMBER);
  g.fillRoundRect(bodyX, bodyY, bodyW, bodyH, radius, SB::ROBOT);
  g.drawRoundRect(bodyX, bodyY, bodyW, bodyH, radius, SB::CYAN);
  g.fillRoundRect(faceX, faceY, faceW, faceH,
                  (w < 110 ? 10 : 19), SB::BG);

  const int leftEye = cx - eyeDX;
  const int rightEye = cx + eyeDX;
  if (mood == RobotMood::BLINK || mood == RobotMood::SLEEPY) {
    g.drawFastHLine(leftEye - eyeW / 2, eyeY, eyeW, SB::CYAN);
    g.drawFastHLine(rightEye - eyeW / 2, eyeY, eyeW, SB::CYAN);
  } else if (mood == RobotMood::HAPPY) {
    g.drawLine(leftEye - eyeW / 2, eyeY + 2, leftEye, eyeY - eyeH / 3, SB::CYAN);
    g.drawLine(leftEye, eyeY - eyeH / 3, leftEye + eyeW / 2, eyeY + 2, SB::CYAN);
    g.drawLine(rightEye - eyeW / 2, eyeY + 2, rightEye, eyeY - eyeH / 3, SB::CYAN);
    g.drawLine(rightEye, eyeY - eyeH / 3, rightEye + eyeW / 2, eyeY + 2, SB::CYAN);
  } else if (mood == RobotMood::FOCUS) {
    g.fillRoundRect(leftEye - eyeW / 2, eyeY - eyeH / 3, eyeW, eyeH * 2 / 3, 4, SB::CYAN);
    g.fillRoundRect(rightEye - eyeW / 2, eyeY - eyeH / 3, eyeW, eyeH * 2 / 3, 4, SB::CYAN);
  } else {
    g.fillRoundRect(leftEye - eyeW / 2, eyeY - eyeH / 2, eyeW, eyeH, eyeW / 2, SB::CYAN);
    g.fillRoundRect(rightEye - eyeW / 2, eyeY - eyeH / 2, eyeW, eyeH, eyeW / 2, SB::CYAN);
  }

  const int mouthY = faceY + faceH * 76 / 100;
  if (mood == RobotMood::HAPPY) {
    g.drawLine(cx - eyeW, mouthY - 2, cx, mouthY + 5, SB::MINT);
    g.drawLine(cx, mouthY + 5, cx + eyeW, mouthY - 2, SB::MINT);
  } else if (mood == RobotMood::SLEEPY) {
    g.drawFastHLine(cx - eyeW / 2, mouthY, eyeW, SB::MINT);
  } else {
    g.drawLine(cx - eyeW / 2, mouthY, cx, mouthY + 3, SB::MINT);
    g.drawLine(cx, mouthY + 3, cx + eyeW / 2, mouthY, SB::MINT);
  }
  if (w > 105) {
    g.fillCircle(faceX + 11, mouthY - 1, 5, SB::EDGE);
    g.fillCircle(faceX + faceW - 11, mouthY - 1, 5, SB::EDGE);
  }
  if (mood == RobotMood::OFFLINE) {
    g.fillCircle(bodyX + bodyW - 9, bodyY + 12, 6, SB::RED);
  }
}
