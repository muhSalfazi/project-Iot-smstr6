#ifndef LIGHT_SENSOR_H
#define LIGHT_SENSOR_H

#include <Arduino.h>

class LightSensor {
  private:
    int analogPin;
    int digitalPin;
    int threshold;

  public:
    // Gunakan threshold 3000 agar nilai di atas itu dianggap cerah
    LightSensor(int aPin, int dPin, int thresh = 3000) {
      analogPin = aPin;
      digitalPin = dPin;
      threshold = thresh;
    }

    void begin() {
      pinMode(analogPin, INPUT);
      pinMode(digitalPin, INPUT);
    }

    int bacaAnalog() {
      return analogRead(analogPin);
    }

    int getAnalogValue() {
      return bacaAnalog();
    }

    // Digital sensor: LOW artinya cerah, HIGH artinya gelap
    bool bacaDigital() {
      int val = digitalRead(digitalPin);
      return val == LOW;  // TRUE = cerah, FALSE = gelap
    }

    bool getDigitalValue() {
      return bacaDigital();
    }

    // Analog: nilai besar = cerah, nilai kecil = gelap
    bool isCerahAnalog() {
      return bacaAnalog() <= threshold;
    }
};

#endif
