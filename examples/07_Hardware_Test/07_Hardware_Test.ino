#include <TFT_eSPI.h>
#include <DHT.h>
#define DHT_PIN 4
#define JOY_X 34
#define JOY_Y 35
#define JOY_SW 32
TFT_eSPI tft;
DHT dht(DHT_PIN, DHT11);
float temperature = NAN, humidity = NAN;
unsigned long lastSensorRead = 0, lastScreenDraw = 0;
void setup() {
  Serial.begin(115200);
  pinMode(JOY_SW, INPUT_PULLUP);
  dht.begin();
  tft.init();
  tft.setRotation(1);
  tft.fillScreen(TFT_BLACK);
  Serial.println("Smart Buddy hardware test!");
}
void loop() {
  if (millis() - lastSensorRead >= 2500) {
    lastSensorRead = millis();
    temperature = dht.readTemperature();
    humidity = dht.readHumidity();
  }
  int x = analogRead(JOY_X), y = analogRead(JOY_Y);
  bool pressed = digitalRead(JOY_SW) == LOW;
  if (millis() - lastScreenDraw >= 250) {
    lastScreenDraw = millis();
    tft.fillScreen(TFT_BLACK);
    tft.setTextDatum(TL_DATUM);
    tft.setTextColor(TFT_CYAN, TFT_BLACK);
    tft.drawString("SMART BUDDY TEST", 20, 15, 4);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.drawString(isnan(temperature) ? "DHT11: No reading" : "Room: " + String(temperature, 1) + " C", 20, 85, 2);
    if (!isnan(humidity)) tft.drawString("Humidity: " + String(humidity, 0) + "%", 20, 135, 2);
    tft.drawString("Joystick X: " + String(x), 20, 195, 2);
    tft.drawString("Joystick Y: " + String(y), 20, 220, 2);
    tft.setTextColor(pressed ? TFT_GREEN : TFT_YELLOW, TFT_BLACK);
    tft.drawString(pressed ? "BUTTON PRESSED" : "BUTTON RELEASED", 20, 255, 2);
  }
  delay(10);
}
