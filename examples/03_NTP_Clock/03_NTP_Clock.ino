#include <WiFi.h>
#include <time.h>
const char* ssid = "YOUR_WIFI_NAME";
const char* password = "YOUR_WIFI_PASSWORD";
void setup() {
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts++ < 30) delay(500);
  if (WiFi.status() != WL_CONNECTED) { Serial.println("Wi-Fi failed"); return; }
  configTime(0, 0, "pool.ntp.org", "time.google.com");
  setenv("TZ", "EST5EDT,M3.2.0/2,M11.1.0/2", 1);
  tzset();
  struct tm t;
  Serial.println(getLocalTime(&t, 20000) ? "Clock synchronized!" : "Clock synchronization failed!");
}
void loop() {
  struct tm t;
  if (getLocalTime(&t, 1000)) {
    Serial.println(&t, "Date: %A, %B %d, %Y");
    Serial.println(&t, "Time: %I:%M:%S %p");
  } else Serial.println("Waiting for clock...");
  delay(1000);
}
