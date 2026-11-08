#include "Weather.h"
#include "Config.h"
#include "RootCA.h"
#include <ArduinoJson.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <time.h>

void WeatherService::begin() {
  updates_ = xQueueCreate(1, sizeof(WeatherReading));
  if (!updates_) {
    Serial.println("[Weather] Queue allocation failed");
    return;
  }
  // Independent task: an HTTPS timeout never freezes drawing or the timer.
  BaseType_t created = xTaskCreatePinnedToCore(
    WeatherService::taskEntry, "weather", 12288, this, 1, &task_, 0);
  if (created != pdPASS) {
    Serial.println("[Weather] Task creation failed");
    vQueueDelete(updates_);
    updates_ = nullptr;
  }
}

bool WeatherService::poll(WeatherReading &reading) {
  if (!updates_) return false;
  return xQueueReceive(updates_, &reading, 0) == pdTRUE;
}

void WeatherService::taskEntry(void *parameter) {
  auto *service = static_cast<WeatherService *>(parameter);
  while (true) {
    if (WiFi.status() == WL_CONNECTED && time(nullptr) > 1700000000) {
      WeatherReading data;
      if (WeatherService::request(data)) {
        xQueueOverwrite(service->updates_, &data);
        vTaskDelay(pdMS_TO_TICKS(SB::WEATHER_PERIOD_MS));
      } else {
        vTaskDelay(pdMS_TO_TICKS(SB::WEATHER_RETRY_MS));
      }
    } else {
      vTaskDelay(pdMS_TO_TICKS(5000));
    }
  }
}

bool WeatherService::request(WeatherReading &result) {
  WiFiClientSecure client;
  client.setCACert(OPEN_METEO_ROOT_CA);  // HTTPS certificate verification ON
  client.setTimeout(6000);
  HTTPClient http;
  http.setTimeout(7000);

  String url = "https://api.open-meteo.com/v1/forecast?latitude=";
  url += String(SB::WEATHER_LATITUDE, 4);
  url += "&longitude=";
  url += String(SB::WEATHER_LONGITUDE, 4);
  url += "&current=temperature_2m,relative_humidity_2m,apparent_temperature,weather_code,wind_speed_10m";
  url += "&timezone=America%2FToronto&forecast_days=1";

  if (!http.begin(client, url)) {
    Serial.println("[Weather] HTTPS begin failed");
    return false;
  }
  int status = http.GET();
  if (status != HTTP_CODE_OK) {
    Serial.printf("[Weather] HTTP/TLS error: %d\n", status);
    http.end();
    return false;
  }

  // This path worked with the user's original ESP32, unlike parsing http.getStream().
  String payload = http.getString();
  http.end();
  JsonDocument doc;
  if (deserializeJson(doc, payload) != DeserializationError::Ok) {
    Serial.println("[Weather] Invalid JSON");
    return false;
  }
  JsonVariant c = doc["current"];
  if (c.isNull() || c["temperature_2m"].isNull() ||
      c["apparent_temperature"].isNull() || c["relative_humidity_2m"].isNull() ||
      c["weather_code"].isNull() || c["wind_speed_10m"].isNull()) {
    Serial.println("[Weather] Missing API fields");
    return false;
  }
  result.valid = true;
  result.temperature = c["temperature_2m"].as<float>();
  result.apparent = c["apparent_temperature"].as<float>();
  result.humidity = c["relative_humidity_2m"].as<int>();
  result.windKmh = c["wind_speed_10m"].as<float>();
  result.code = c["weather_code"].as<int>();
  result.updatedMs = millis();
  Serial.printf("[Weather] OK: %.1f C, code %d\n", result.temperature, result.code);
  return true;
}
