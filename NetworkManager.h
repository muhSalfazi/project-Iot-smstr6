#ifndef NETWORKMANAGER_H
#define NETWORKMANAGER_H

#include <WiFi.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <RTClib.h>  // Untuk DateTime

class NetworkManager {
  private:
    const char* ssid = "Galaxy";
    const char* password = "#pohobanget";
    WiFiUDP ntpUDP;
    NTPClient timeClient;
    
  public:
    NetworkManager() : timeClient(ntpUDP, "pool.ntp.org", 25200, 60000) {
      // 25200 adalah offset waktu untuk GMT+7 (dalam detik)
    }

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
        timeClient.begin();
        timeClient.update();
      } else {
        Serial.println("\nGagal konek ke WiFi!");
      }
    }

    DateTime getNTPTime() {
      if (WiFi.status() == WL_CONNECTED) {
        timeClient.update();
        unsigned long epochTime = timeClient.getEpochTime();
        return DateTime((uint32_t)epochTime);
      }
      return DateTime(0, 0, 0); // Return tanggal 0/0/0 jika tidak terhubung
    }

    bool isConnected() {
      return WiFi.status() == WL_CONNECTED;
    }
};

#endif