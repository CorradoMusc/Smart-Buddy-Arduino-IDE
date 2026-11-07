// Smart Buddy V2 — reduced flicker: draw static screen only on page changes,
// and update individual data fields without clearing the entire 480x320 LCD.
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <TFT_eSPI.h>
#include <DHT.h>
#include <time.h>
#include <math.h>

const char* ssid = "YOUR_WIFI_NAME";
const char* password = "YOUR_WIFI_PASSWORD";
const char* timeZone = "EST5EDT,M3.2.0/2,M11.1.0/2";
const char* weatherURL = "https://api.open-meteo.com/v1/forecast?latitude=43.8561&longitude=-79.3370&current=temperature_2m,relative_humidity_2m,apparent_temperature,weather_code&timezone=America%2FToronto";
#define DHT_PIN 4
#define JOY_X 34
#define JOY_Y 35
#define JOY_SW 32
#define BG TFT_BLACK
TFT_eSPI tft;
DHT dht(DHT_PIN,DHT11);

float outdoorTemp=NAN, feelsLike=NAN, indoorTemp=NAN, indoorHumidity=NAN;
int outdoorHumidity=-1, weatherCode=-1;
bool weatherValid=false;
unsigned long lastWeatherSuccess=0;
int page=0;
bool joystickReady=true, buttonStable=false, buttonLastRaw=false;
unsigned long buttonChanged=0;
const unsigned long focusDuration=25UL*60UL*1000UL;
unsigned long timerRemaining=focusDuration, lastTimerTick=0;
bool timerRunning=false;
unsigned long lastWeather=0,lastDHT=0,lastDraw=0,lastWiFiCheck=0;
const unsigned long weatherInterval=900000UL, weatherRetry=60000UL;
bool pageNeedsDraw=true, dataNeedsDraw=true;

String weatherDescription(int code) {
  if(code==0) return "Clear sky";
  if(code<=3) return "Cloudy";
  if(code<=48) return "Fog";
  if(code<=67) return "Rain";
  if(code<=77) return "Snow";
  if(code<=82) return "Rain showers";
  if(code<=86) return "Snow showers";
  if(code<=99) return "Thunderstorm";
  return "Unknown";
}

bool fetchWeather() {
  if(WiFi.status()!=WL_CONNECTED) {Serial.println("Weather skipped: Wi-Fi offline");return false;}
  WiFiClientSecure client;
  client.setInsecure(); // TEST BUILD ONLY: no server certificate verification.
  HTTPClient http;
  http.setTimeout(8000);
  if(!http.begin(client,weatherURL)) return false;
  bool success=false;
  int status=http.GET();
  if(status==HTTP_CODE_OK) {
    String payload=http.getString(); // Avoid stream parsing issue seen in original test.
    JsonDocument doc;
    if(deserializeJson(doc,payload)==DeserializationError::Ok && doc["current"].is<JsonObject>()) {
      JsonObject c=doc["current"];
      if(!c["temperature_2m"].isNull() && !c["apparent_temperature"].isNull() &&
         !c["relative_humidity_2m"].isNull() && !c["weather_code"].isNull()) {
        outdoorTemp=c["temperature_2m"].as<float>();
        feelsLike=c["apparent_temperature"].as<float>();
        outdoorHumidity=c["relative_humidity_2m"].as<int>();
        weatherCode=c["weather_code"].as<int>();
        weatherValid=true;lastWeatherSuccess=millis();success=true;dataNeedsDraw=true;
        Serial.println("Weather updated!");
      }
    } else Serial.println("Weather JSON error");
  } else Serial.printf("Weather HTTP error: %d\n",status);
  http.end();
  return success;
}

// Clear a fixed region only: used for changing text within the page.
void field(int x,int y,int w,int h,const String& value,uint16_t color,int font) {
  tft.fillRect(x,y,w,h,BG);
  tft.setTextDatum(TL_DATUM);
  tft.setTextColor(color,BG);
  tft.drawString(value,x,y,font);
}
void drawPage() {
  tft.fillScreen(BG); // Only when switching between pages.
  tft.setTextDatum(TL_DATUM);
  tft.setTextColor(TFT_CYAN,BG);
  tft.drawString("SMART BUDDY",16,12,4);
  tft.drawFastHLine(16,48,448,TFT_DARKGREY);
  const char* titles[]={"HOME","WEATHER","ROOM","STUDY TIMER"};
  tft.setTextColor(TFT_YELLOW,BG);
  tft.drawString(titles[page],20,65,2);
  tft.drawFastHLine(16,290,448,TFT_DARKGREY);
  tft.setTextColor(TFT_GREEN,BG);
  tft.drawCentreString("<  "+String(page+1)+" / 4  >",240,298,2);
  dataNeedsDraw=true;
}
void drawDynamic() {
  struct tm info;
  if(page==0) {
    if(getLocalTime(&info,50)) {
      char buf[48];
      strftime(buf,sizeof(buf),"%I:%M:%S %p",&info);
      field(20,105,440,38,buf,TFT_WHITE,4);
      strftime(buf,sizeof(buf),"%A, %B %d",&info);
      field(20,155,440,28,buf,TFT_WHITE,2);
    } else field(20,105,440,38,"Syncing clock...",TFT_WHITE,2);
    field(20,200,440,38,weatherValid ? "Outside: "+String(outdoorTemp,1)+" C" : "Weather unavailable",TFT_WHITE,weatherValid?4:2);
    field(20,255,440,28,WiFi.status()==WL_CONNECTED ? "Wi-Fi connected" : "Wi-Fi offline",WiFi.status()==WL_CONNECTED?TFT_GREEN:TFT_RED,2);
  } else if(page==1) {
    if(weatherValid) {
      field(20,105,440,38,String(outdoorTemp,1)+" C",TFT_WHITE,4);
      field(20,155,440,38,weatherDescription(weatherCode),TFT_WHITE,4);
      field(20,215,440,28,"Feels like: "+String(feelsLike,1)+" C",TFT_WHITE,2);
      field(20,245,440,28,"Humidity: "+String(outdoorHumidity)+"%",TFT_WHITE,2);
    } else field(20,125,440,38,"Waiting for weather...",TFT_WHITE,2);
  } else if(page==2) {
    if(!isnan(indoorTemp)&&!isnan(indoorHumidity)) {
      field(20,110,440,38,"Temperature: "+String(indoorTemp,1)+" C",TFT_WHITE,4);
      field(20,170,440,38,"Humidity: "+String(indoorHumidity,0)+"%",TFT_WHITE,4);
    } else field(20,130,440,38,"Sensor unavailable",TFT_WHITE,2);
  } else {
    unsigned long seconds=(timerRemaining+999UL)/1000UL;
    char buf[16]; snprintf(buf,sizeof(buf),"%02lu:%02lu",seconds/60,seconds%60);
    field(20,115,330,64,buf,TFT_CYAN,7);
    String status=timerRunning ? "FOCUSING" : timerRemaining==0 ? "SESSION COMPLETE!" :
       timerRemaining==focusDuration ? "READY TO START" : "PAUSED";
    field(20,225,440,25,status,TFT_WHITE,2);
    field(20,255,440,26,"Press joystick to start/pause",TFT_WHITE,2);
  }
}
void readIndoor() {
  float t=dht.readTemperature(), h=dht.readHumidity();
  if(!isnan(t)&&!isnan(h)) {
    if(isnan(indoorTemp)||isnan(indoorHumidity)||t!=indoorTemp||h!=indoorHumidity) {
      indoorTemp=t;indoorHumidity=h;
      if(page==2) dataNeedsDraw=true;
    }
  } else Serial.println("DHT11 read failed");
}
void handleJoystick() {
  int x=analogRead(JOY_X);
  if(joystickReady && x>3300){page=(page+1)%4;joystickReady=false;pageNeedsDraw=true;}
  else if(joystickReady && x<800){page=(page+3)%4;joystickReady=false;pageNeedsDraw=true;}
  if(x>1400&&x<2700) joystickReady=true;
  bool raw=digitalRead(JOY_SW)==LOW;
  if(raw!=buttonLastRaw){buttonChanged=millis();buttonLastRaw=raw;}
  if(millis()-buttonChanged>=50 && raw!=buttonStable){
    buttonStable=raw;
    if(buttonStable && page==3){
      if(timerRemaining==0) timerRemaining=focusDuration;
      timerRunning=!timerRunning;lastTimerTick=millis();dataNeedsDraw=true;
    }
  }
}
void updateTimer(unsigned long now) {
  if(!timerRunning) return;
  unsigned long elapsed=now-lastTimerTick;lastTimerTick=now;
  if(elapsed>=timerRemaining){timerRemaining=0;timerRunning=false;dataNeedsDraw=true;Serial.println("Study session complete!");}
  else timerRemaining-=elapsed;
}
void setup() {
  Serial.begin(115200);
  pinMode(JOY_SW,INPUT_PULLUP);
  dht.begin();tft.init();tft.setRotation(1);tft.fillScreen(BG);
  tft.setTextColor(TFT_CYAN,BG);tft.drawString("Starting Smart Buddy...",20,90,4);
  WiFi.mode(WIFI_STA);WiFi.setAutoReconnect(true);WiFi.begin(ssid,password);
  unsigned long start=millis();
  while(WiFi.status()!=WL_CONNECTED&&millis()-start<15000UL) delay(100);
  configTzTime(timeZone,"pool.ntp.org","time.google.com");
  lastTimerTick=millis();
  // First weather attempt happens immediately in the loop.
  lastWeather=millis()-weatherInterval;
  pageNeedsDraw=true;dataNeedsDraw=true;
}
void loop() {
  unsigned long now=millis();
  if(now-lastWiFiCheck>=10000UL){
    lastWiFiCheck=now;
    if(WiFi.status()!=WL_CONNECTED) WiFi.reconnect();
    if(page==0)dataNeedsDraw=true;
  }
  if(now-lastDHT>=2500UL){lastDHT=now;readIndoor();}
  if(now-lastWeather>=weatherInterval){
    lastWeather=millis();
    if(!fetchWeather()) lastWeather=millis()-(weatherInterval-weatherRetry);
  }
  handleJoystick();
  updateTimer(millis());
  if(pageNeedsDraw){drawPage();pageNeedsDraw=false;dataNeedsDraw=true;}
  // Home clock and timer need updates every second, without a full page clear.
  if(dataNeedsDraw || ((page==0 || page==3) && millis()-lastDraw>=1000UL)){
    drawDynamic();dataNeedsDraw=false;lastDraw=millis();
  }
  delay(20);
}
