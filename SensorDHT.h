#ifndef SENSORDHT_H
#define SENSORDHT_H

#include <DHT.h>

class SensorDHT {
  private:
    DHT dht;
    float temperature;
    float humidity;
    float heatIndex;
    float dewPoint;

  public:
    SensorDHT(uint8_t pin, uint8_t type) : dht(pin, type) {}
    
    void begin() {
      dht.begin();
    }
    
    void bacaData() {
      temperature = dht.readTemperature();
      humidity = dht.readHumidity();
      
      if (!isnan(temperature) && !isnan(humidity)) {
        heatIndex = dht.computeHeatIndex(temperature, humidity, false);
        dewPoint = temperature - ((100 - humidity) / 5);
      }
    }
    
    float getTemperature() { return temperature; }
    float getHumidity() { return humidity; }
    float getHeatIndex() { return heatIndex; }
    float getDewPoint() { return dewPoint; }
    
    bool isDataValid() {
      return !isnan(temperature) && !isnan(humidity);
    }
};

#endif