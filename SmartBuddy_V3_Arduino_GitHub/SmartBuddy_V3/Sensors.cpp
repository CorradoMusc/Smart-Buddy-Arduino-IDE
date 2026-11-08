#include "Sensors.h"
#include "Config.h"

Sensors::Sensors() : dht_(SB::DHT_PIN, DHT11) {}
void Sensors::begin() { dht_.begin(); lastPoll_ = millis() - SB::SENSOR_PERIOD_MS; }
void Sensors::poll(uint32_t now) {
  if ((uint32_t)(now - lastPoll_) < SB::SENSOR_PERIOD_MS) return;
  lastPoll_ = now;
  float t = dht_.readTemperature();
  float h = dht_.readHumidity();
  if (!isnan(t) && !isnan(h) && h >= 0.0f && h <= 100.0f) {
    data_.temperature = t;
    data_.humidity = h;
    data_.valid = true;
    data_.updatedMs = now;
  } else {
    Serial.println("[DHT11] Read failed; retaining last good value");
  }
}
