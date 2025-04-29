#ifndef JEMURANSERVO_H
#define JEMURANSERVO_H

#include <ESP32Servo.h>

class JemuranServo {
  private:
    Servo servo;
    uint8_t pin;
    bool isTerbuka;
    const int POSISI_TERTUTUP = 0;
    const int POSISI_TERBUKA = 90;

  public:
    JemuranServo(uint8_t servoPin) : pin(servoPin) {}
    
    void begin() {
      servo.attach(pin);
      tutup();
    }
    
    void buka() {
      servo.write(POSISI_TERBUKA);
      isTerbuka = true;
    }
    
    void tutup() {
      servo.write(POSISI_TERTUTUP);
      isTerbuka = false;
    }
    
    bool getStatus() { return isTerbuka; }
};

#endif