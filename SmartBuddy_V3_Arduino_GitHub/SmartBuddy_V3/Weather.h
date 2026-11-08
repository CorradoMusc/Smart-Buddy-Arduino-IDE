#pragma once
#include "Model.h"
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>
#include <freertos/task.h>

class WeatherService {
public:
  void begin();
  bool poll(WeatherReading &reading);
private:
  static void taskEntry(void *parameter);
  static bool request(WeatherReading &result);
  QueueHandle_t updates_ = nullptr;
  TaskHandle_t task_ = nullptr;
};
