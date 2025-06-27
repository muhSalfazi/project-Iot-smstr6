#ifndef RTCMANAGER_H
#define RTCMANAGER_H

#include <RTClib.h>

// Forward declaration
class NetworkManager;

class RTCManager {
  private:
    RTC_DS3231 rtc;
    NetworkManager& networkManager;
    
  public:
    // Constructor dengan reference ke NetworkManager
    explicit RTCManager(NetworkManager& nm) : networkManager(nm) {}

    bool begin() {
      if (!rtc.begin()) {
        Serial.println("RTC tidak terdeteksi!");
        return false;
      }

      if (rtc.lostPower() || networkManager.isConnected()) {
        Serial.println("Mengatur ulang waktu RTC...");
        if (networkManager.isConnected()) {
          DateTime ntpTime = networkManager.getNTPTime();
          if (ntpTime.year() >= 2023) {
            rtc.adjust(ntpTime);
            Serial.println("Waktu RTC diatur dari NTP");
          } else {
            rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
            Serial.println("Waktu RTC diatur dari waktu kompilasi");
          }
        } else {
          rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
          Serial.println("Waktu RTC diatur dari waktu kompilasi");
        }
      }
      return true;
    }

    String getFormattedTime() {
      DateTime now = rtc.now();
      char buffer[20];
      sprintf(buffer, "%02d:%02d:%02d %02d/%02d/%04d", 
              now.hour(), now.minute(), now.second(),
              now.day(), now.month(), now.year());
      return String(buffer);
    }

    DateTime now() {
      return rtc.now();
    }
};

#endif