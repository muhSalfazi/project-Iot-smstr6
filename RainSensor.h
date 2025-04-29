#ifndef RAINSENSOR_H
#define RAINSENSOR_H

class RainSensor {
  private:
    uint8_t pinDigital;
    uint8_t pinAnalog;
    bool isRaining;
    int intensity;
    String rainType;

  public:
    RainSensor(uint8_t digPin, uint8_t anaPin) : 
      pinDigital(digPin), pinAnalog(anaPin) {}
    
    void begin() {
      pinMode(pinDigital, INPUT);
    }
    
    void bacaData() {
      isRaining = (digitalRead(pinDigital) == LOW);
      intensity = analogRead(pinAnalog);
      
      if (isRaining) {
        if (intensity > 500) rainType = "Gerimis";
        else if (intensity > 200) rainType = "Hujan Sedang";
        else rainType = "Hujan Lebat";
      } else {
        rainType = "Tidak Hujan";
      }
    }
    
    bool getIsRaining() { return isRaining; }
    int getIntensity() { return intensity; }
    String getRainType() { return rainType; }
};

#endif