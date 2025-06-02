#ifndef BUZZER_H
#define BUZZER_H

#include <Arduino.h>

class Buzzer {
  private:
    uint8_t pin;
    uint8_t channel = 5;      
    uint16_t freq = 1000;
    uint8_t resolution = 8;

  public:
    Buzzer(uint8_t _pin) {
      pin = _pin;
    }

    void begin() {
      ledcSetup(channel, freq, resolution);
      ledcAttachPin(pin, channel);
      off();
    }

    void on() {
      ledcWrite(channel, 128);
    }

    void off() {
      ledcWrite(channel, 0);
    }

    void beep(int duration = 300) {
      on();
      delay(duration);
      off();
    }
};

#endif