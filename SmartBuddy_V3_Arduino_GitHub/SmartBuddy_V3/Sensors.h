#pragma once
#include <DHT.h>
#include "Model.h"

class Sensors {
public:
  Sensors();
  void begin();
  void poll(uint32_t now);
  RoomReading snapshot() const { return data_; }
private:
  DHT dht_;
  RoomReading data_;
  uint32_t lastPoll_ = 0;
};
