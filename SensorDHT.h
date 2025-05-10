#ifndef SENSORDHT_H
#define SENSORDHT_H

#include <DHT.h>

class SensorDHT {
  private:
    DHT dht;
    float temperature = NAN;
    float humidity = NAN;
    float heatIndex = NAN;
    float dewPoint = NAN;

  public:
    SensorDHT(uint8_t pin, uint8_t type) : dht(pin, type) {}

    void begin() {
      dht.begin();
    }

    void bacaData() {
      float t = dht.readTemperature();
      float h = dht.readHumidity();

      // Validasi hasil baca
      if (!isnan(t) && !isnan(h)) {
        temperature = t;
        humidity = h;
        heatIndex = dht.computeHeatIndex(temperature, humidity, false);
        dewPoint = temperature - ((100 - humidity) / 5.0); // Rumus perkiraan dew point
      } else {
        // Gagal baca, reset nilai ke NAN
        temperature = humidity = heatIndex = dewPoint = NAN;
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
