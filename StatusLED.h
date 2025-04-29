#ifndef STATUSLED_H
#define STATUSLED_H

class StatusLED {
  private:
    uint8_t pinHijau;
    uint8_t pinMerah;
    unsigned long previousMillis;
    int ledState;
    int blinkInterval;

  public:
    StatusLED(uint8_t hijau, uint8_t merah) : 
      pinHijau(hijau), pinMerah(merah), blinkInterval(500), ledState(LOW) {}
    
    void begin() {
      pinMode(pinHijau, OUTPUT);
      pinMode(pinMerah, OUTPUT);
      matikanSemua();
    }
    
    void setKondisiIdeal(bool ideal) {
      if (ideal) {
        digitalWrite(pinHijau, HIGH);
        digitalWrite(pinMerah, LOW);
      } else {
        digitalWrite(pinHijau, LOW);
      }
    }
    
    void kedipMerah(bool isRaining, int intensitas) {
      if (!isRaining) {
        blinkInterval = 500; // Kedip pelan saat tidak ideal
      } else {
        if (intensitas > 500) blinkInterval = 1000;
        else if (intensitas > 200) blinkInterval = 500;
        else blinkInterval = 200;
      }
      
      unsigned long currentMillis = millis();
      if (currentMillis - previousMillis >= blinkInterval) {
        previousMillis = currentMillis;
        ledState = !ledState;
        digitalWrite(pinMerah, ledState);
      }
    }
    
    void matikanSemua() {
      digitalWrite(pinHijau, LOW);
      digitalWrite(pinMerah, LOW);
    }
    
    void error() {
      digitalWrite(pinMerah, HIGH);
    }
};

#endif