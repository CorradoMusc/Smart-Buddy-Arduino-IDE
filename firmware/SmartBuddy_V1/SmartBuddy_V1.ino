// Smart Buddy V1 — initial functional prototype (full-screen redraw every second)
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <TFT_eSPI.h>
#include <DHT.h>
#include <time.h>
const char* ssid = "YOUR_WIFI_NAME";
const char* password = "YOUR_WIFI_PASSWORD";
const char* weatherURL = "https://api.open-meteo.com/v1/forecast?latitude=43.8561&longitude=-79.3370&current=temperature_2m,relative_humidity_2m,apparent_temperature,weather_code&timezone=America%2FToronto";
#define DHT_PIN 4
#define JOY_X 34
#define JOY_Y 35
#define JOY_SW 32
TFT_eSPI tft;
DHT dht(DHT_PIN, DHT11);
float outside = NAN, feel = NAN, inside = NAN, humidity = NAN;
int outdoorHumidity = -1, weatherCode = -1;
int page = 0;
bool stickCentered = true, previousButton = false, running = false;
const unsigned long focusDuration = 1500000UL;
unsigned long remaining = focusDuration, lastTick = 0, lastWeather = 0, lastDHT = 0, lastDraw = 0, lastWiFi = 0;
bool redraw = true;
String conditions(int code) {
  if (code == 0) return "Clear";
  if (code <= 3) return "Cloudy";
  if (code <= 48) return "Fog";
  if (code <= 67) return "Rain";
  if (code <= 77) return "Snow";
  if (code <= 82) return "Showers";
  if (code <= 86) return "Snow showers";
  if (code <= 99) return "Thunderstorm";
  return "Unknown";
}
void fetchWeather() {
  if (WiFi.status() != WL_CONNECTED) return;
  WiFiClientSecure client;
  client.setInsecure(); // PROTOTYPE ONLY: no certificate verification.
  HTTPClient http;
  http.setTimeout(10000);
  if (!http.begin(client, weatherURL)) return;
  if (http.GET() == HTTP_CODE_OK) {
    JsonDocument doc;
    String payload = http.getString();
    if (deserializeJson(doc, payload) == DeserializationError::Ok && doc["current"].is<JsonObject>()) {
      JsonObject c = doc["current"];
      if (!c["temperature_2m"].isNull()) {
        outside = c["temperature_2m"].as<float>();
        feel = c["apparent_temperature"].as<float>();
        outdoorHumidity = c["relative_humidity_2m"].as<int>();
        weatherCode = c["weather_code"].as<int>();
        redraw = true;
      }
    }
  }
  http.end();
}
void drawScreen() {
  tft.fillScreen(TFT_BLACK); // Intentionally preserved V1 behaviour; causes flicker.
  tft.setTextDatum(TL_DATUM);
  tft.setTextColor(TFT_CYAN, TFT_BLACK);
  tft.drawString("SMART BUDDY", 16, 12, 4);
  tft.drawFastHLine(16, 48, 448, TFT_DARKGREY);
  const char* titles[] = {"HOME", "WEATHER", "ROOM", "STUDY TIMER"};
  tft.setTextColor(TFT_YELLOW, TFT_BLACK);
  tft.drawString(titles[page], 20, 65, 2);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  if (page == 0) {
    struct tm timeinfo;
    if (getLocalTime(&timeinfo, 50)) {
      char buf[48];
      strftime(buf, sizeof(buf), "%I:%M:%S %p", &timeinfo);
      tft.drawString(buf, 20, 105, 4);
      strftime(buf, sizeof(buf), "%A, %B %d", &timeinfo);
      tft.drawString(buf, 20, 155, 2);
    }
    tft.drawString(isnan(outside) ? "Weather unavailable" : "Outside: " + String(outside,1) + " C", 20, 200, 4);
    tft.drawString(WiFi.status() == WL_CONNECTED ? "Wi-Fi connected" : "Wi-Fi offline",20,255,2);
  } else if (page == 1) {
    if (!isnan(outside)) {
      tft.drawString(String(outside,1)+" C",20,105,4);
      tft.drawString(conditions(weatherCode),20,155,4);
      tft.drawString("Feels like: "+String(feel,1)+" C",20,215,2);
      tft.drawString("Humidity: "+String(outdoorHumidity)+"%",20,245,2);
    } else tft.drawString("Waiting for weather...",20,125,2);
  } else if (page == 2) {
    if (!isnan(inside) && !isnan(humidity)) {
      tft.drawString("Temperature: "+String(inside,1)+" C",20,110,4);
      tft.drawString("Humidity: "+String(humidity,0)+"%",20,170,4);
    } else tft.drawString("Sensor unavailable",20,130,2);
  } else {
    unsigned long seconds = (remaining+999UL)/1000UL;
    char buffer[16];
    snprintf(buffer,sizeof(buffer),"%02lu:%02lu",seconds/60,seconds%60);
    tft.setTextColor(TFT_CYAN,TFT_BLACK);
    tft.drawString(buffer,20,115,7);
    tft.setTextColor(TFT_WHITE,TFT_BLACK);
    tft.drawString(running ? "FOCUSING" : "PAUSED / READY",20,225,2);
    tft.drawString("Press joystick to start/pause",20,255,2);
  }
  tft.drawFastHLine(16,290,448,TFT_DARKGREY);
  tft.setTextColor(TFT_GREEN,TFT_BLACK);
  tft.drawString("<  "+String(page+1)+" / 4  >",195,299,2);
}
void setup() {
  Serial.begin(115200);
  pinMode(JOY_SW, INPUT_PULLUP);
  dht.begin();
  tft.init(); tft.setRotation(1); tft.fillScreen(TFT_BLACK);
  WiFi.mode(WIFI_STA); WiFi.begin(ssid,password);
  unsigned long start = millis();
  while(WiFi.status()!=WL_CONNECTED && millis()-start<15000UL) delay(250);
  configTzTime("EST5EDT,M3.2.0/2,M11.1.0/2","pool.ntp.org","time.google.com");
  fetchWeather();
  lastWeather = lastTick = millis();
}
void loop() {
  unsigned long now = millis();
  if (now-lastWiFi>=10000UL) { lastWiFi=now; if(WiFi.status()!=WL_CONNECTED) WiFi.reconnect(); }
  if (now-lastDHT>=2500UL) { lastDHT=now; float t=dht.readTemperature(),h=dht.readHumidity(); if(!isnan(t)&&!isnan(h)){inside=t;humidity=h;} }
  if (now-lastWeather>=900000UL) {lastWeather=now;fetchWeather();}
  int x=analogRead(JOY_X);
  if (stickCentered && x>3300) {page=(page+1)%4;stickCentered=false;redraw=true;}
  else if (stickCentered && x<800) {page=(page+3)%4;stickCentered=false;redraw=true;}
  if(x>1400 && x<2700) stickCentered=true;
  bool button=digitalRead(JOY_SW)==LOW;
  if (button && !previousButton && page==3) {
    if (remaining==0) remaining=focusDuration;
    running=!running;lastTick=millis();redraw=true;
  }
  previousButton=button;
  now=millis();
  if (running) {
    unsigned long elapsed=now-lastTick;lastTick=now;
    if(elapsed>=remaining){remaining=0;running=false;redraw=true;}
    else remaining-=elapsed;
  }
  if(redraw || millis()-lastDraw>=1000UL){lastDraw=millis();drawScreen();redraw=false;}
  delay(20);
}
