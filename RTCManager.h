#ifndef RTCMANAGER_H
#define RTCMANAGER_H

#include <RTClib.h>

class RTCManager {
  private:
    RTC_DS3231 rtc;

  public:
    bool begin() {
      if (!rtc.begin()) {
        Serial.println("RTC tidak terdeteksi!");
        return false;
      }

      if (rtc.lostPower()) {
        Serial.println("RTC kehilangan daya, atur ulang waktu...");
        // Set waktu ke saat ini (hanya dilakukan sekali)
        rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
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
