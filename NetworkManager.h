#ifndef NETWORKMANAGER_H
#define NETWORKMANAGER_H

#include <WiFi.h>

class NetworkManager {
  private:
    const char* ssid = "Galaxy";
    const char* password = "#pohobanget";
    
  public:
    void connectToWiFi() {
      Serial.print("Menghubungkan ke ");
      Serial.println(ssid);

      WiFi.begin(ssid, password);

      int attempt = 0;
      while (WiFi.status() != WL_CONNECTED && attempt < 30) {
        delay(500);
        Serial.print(".");
        attempt++;
      }

      if (WiFi.status() == WL_CONNECTED) {
        Serial.println("\nWiFi connected!!");
        Serial.println("IP address: ");
        Serial.println(WiFi.localIP());
      } else {
        Serial.println("\nGagal konek ke WiFi!");
      }
    }

};

#endif