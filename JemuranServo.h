#ifndef JEMURANSERVO_H
#define JEMURANSERVO_H

#include <ESP32Servo.h>

class JemuranServo {
  private:
    Servo servo;
    uint8_t pin;
    bool isTerbuka;
    const int POSISI_TERTUTUP = 0;    // 0 derajat
    const int POSISI_TERBUKA = 90;    // 90 derajat

    void logAction(const char* action) {
      Serial.print("[SERVO] ");
      Serial.print(action);
      Serial.print(" | Status: ");
      Serial.println(isTerbuka ? "TERBUKA" : "TERTUTUP");
    }

  public:
    JemuranServo(uint8_t servoPin) : pin(servoPin), isTerbuka(false) {}
    
    void begin() {
      servo.attach(pin);
      servo.write(POSISI_TERTUTUP);
      logAction("Inisialisasi");
    }
    
    void buka() {
        servo.write(POSISI_TERBUKA);
        isTerbuka = true;
        logAction("Membuka");
    }
    
    void tutup() {
        servo.write(POSISI_TERTUTUP);
        isTerbuka = false;
        logAction("Menutup");
    }
    
    bool getStatus() { 
      return isTerbuka; 
    }

    // Handler untuk pesan MQTT
      void handleMQTTMessage(const char* command) {
      if (strcmp(command, "buka") == 0) {
        buka();
        Serial.println("MQTT: Jemuran dibuka");
      } else if (strcmp(command, "tutup") == 0) {
        tutup();
        Serial.println("MQTT: Jemuran ditutup");
      }
    }

};

#endif