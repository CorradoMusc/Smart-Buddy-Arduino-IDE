#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <time.h>
const char* ssid = "YOUR_WIFI_NAME";
const char* password = "YOUR_WIFI_PASSWORD";
const char* weatherURL = "https://api.open-meteo.com/v1/forecast?latitude=43.8561&longitude=-79.3370&current=temperature_2m,relative_humidity_2m,apparent_temperature,weather_code,wind_speed_10m&timezone=America%2FToronto";
unsigned long lastWeather = 0;
void fetchWeather() {
  if (WiFi.status() != WL_CONNECTED) { Serial.println("Wi-Fi offline"); return; }
  WiFiClientSecure client;
  client.setInsecure(); // TEST ONLY: replace with CA verification before permanent deployment.
  HTTPClient http;
  http.setTimeout(10000);
  if (!http.begin(client, weatherURL)) { Serial.println("Request setup failed"); return; }
  int status = http.GET();
  if (status == HTTP_CODE_OK) {
    // Reading into a String fixed the original InvalidInput stream-parsing issue.
    String payload = http.getString();
    JsonDocument doc;
    if (deserializeJson(doc, payload) == DeserializationError::Ok && doc["current"].is<JsonObject>()) {
      JsonObject c = doc["current"];
      Serial.printf("Temperature: %.1f C\n", c["temperature_2m"].as<float>());
      Serial.printf("Feels like: %.1f C\n", c["apparent_temperature"].as<float>());
      Serial.printf("Humidity: %d%%\n", c["relative_humidity_2m"].as<int>());
      Serial.printf("Wind: %.1f km/h\n", c["wind_speed_10m"].as<float>());
      Serial.printf("Weather code: %d\n", c["weather_code"].as<int>());
    } else Serial.println("Weather JSON error");
  } else Serial.printf("HTTP error: %d\n", status);
  http.end();
}
void setup() {
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts++ < 40) delay(500);
  configTzTime("EST5EDT,M3.2.0/2,M11.1.0/2", "pool.ntp.org", "time.google.com");
  fetchWeather();
  lastWeather = millis();
}
void loop() {
  struct tm t;
  if (getLocalTime(&t, 100)) Serial.println(&t, "%I:%M:%S %p");
  if (millis() - lastWeather >= 900000UL) { lastWeather = millis(); fetchWeather(); }
  delay(1000);
}
