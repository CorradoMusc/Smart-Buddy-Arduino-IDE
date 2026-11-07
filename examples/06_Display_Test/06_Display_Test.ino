#include <TFT_eSPI.h>
TFT_eSPI tft;
void setup() {
  Serial.begin(115200);
  tft.init();
  tft.setRotation(1);
  tft.fillScreen(TFT_BLACK);
  tft.setTextDatum(MC_DATUM);
  tft.setTextColor(TFT_CYAN, TFT_BLACK);
  tft.drawString("SMART BUDDY", 240, 110, 4);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.drawString("Display Connected!", 240, 180, 2);
  tft.setTextColor(TFT_GREEN, TFT_BLACK);
  tft.drawString("ESP32 + ST7796S", 240, 220, 2);
  Serial.println("Display commands sent!");
}
void loop() {}
