#include <DHT.h>
#define DHT_PIN 4
#define DHT_TYPE DHT11
DHT dht(DHT_PIN, DHT_TYPE);
void setup() { Serial.begin(115200); dht.begin(); delay(2000); }
void loop() {
  float t = dht.readTemperature();
  float h = dht.readHumidity();
  if (isnan(t) || isnan(h)) Serial.println("DHT11 reading failed!");
  else {
    Serial.printf("Room temperature: %.1f C\n", t);
    Serial.printf("Room humidity: %.0f%%\n", h);
  }
  delay(2500);
}
