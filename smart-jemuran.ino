#include "SensorDHT.h"
#include "RainSensor.h"
#include "JemuranServo.h"
#include "StatusLED.h"

#define DHTPIN 17
#define RAINPIN_DIGITAL 15
#define RAINPIN_ANALOG 35
#define REDLED 18
#define GREENLED 19
#define SERVO_PIN 21

SensorDHT sensorDHT(DHTPIN, DHT11);
RainSensor rainSensor(RAINPIN_DIGITAL, RAINPIN_ANALOG);
JemuranServo jemuran(SERVO_PIN);
StatusLED statusLED(GREENLED, REDLED);

void setup() {
  Serial.begin(115200);
  sensorDHT.begin();
  rainSensor.begin();
  jemuran.begin();
  statusLED.begin();
}

void tampilkanData(bool isIdeal) {  // Tambahkan parameter isIdeal
  Serial.println("\n========== DATA ==========");
  Serial.print("Suhu: "); Serial.print(sensorDHT.getTemperature(), 1); Serial.println(" °C");
  Serial.print("Kelembapan: "); Serial.print(sensorDHT.getHumidity(), 1); Serial.println(" %");
  Serial.print("Indeks Panas: "); Serial.print(sensorDHT.getHeatIndex(), 1); Serial.println(" °C");
  Serial.print("Titik Embun: "); Serial.print(sensorDHT.getDewPoint(), 1); Serial.println(" °C");
  Serial.print("Status Hujan: "); Serial.println(rainSensor.getRainType());
  Serial.print("Kondisi Jemur: "); Serial.println(isIdeal ? "IDEAL" : "TIDAK IDEAL");
  Serial.println("========================");
}

void loop() {
  // Baca semua sensor
  sensorDHT.bacaData();
  rainSensor.bacaData();
  
  // Handle error sensor
  if (!sensorDHT.isDataValid()) {
    Serial.println("Gagal baca DHT11!");
    statusLED.error();
    return;
  }

  // Logika kondisi jemur
  bool isIdeal = !rainSensor.getIsRaining() && 
                (sensorDHT.getTemperature() >= 28) && 
                (sensorDHT.getTemperature() <= 35) && 
                (sensorDHT.getHumidity() <= 70) && 
                (sensorDHT.getHeatIndex() < 40) && 
                ((sensorDHT.getTemperature() - sensorDHT.getDewPoint()) > 2);

  // Kontrol LED
  statusLED.setKondisiIdeal(isIdeal);
  if (!isIdeal) {
    statusLED.kedipMerah(rainSensor.getIsRaining(), rainSensor.getIntensity());
  }

  // Kontrol Servo Otomatis
  if (rainSensor.getIsRaining() && jemuran.getStatus()) {
    jemuran.tutup();
    Serial.println("WARNING: Hujan terdeteksi! Jemuran ditutup otomatis");
  }

  // Kontrol Manual via Serial
  if (Serial.available()) {
    char input = Serial.read();
    
    if (input == 'b' && !rainSensor.getIsRaining()) {
      jemuran.buka();
      Serial.println("Jemuran dibuka");
    } 
    else if (input == 't') {
      jemuran.tutup();
      Serial.println("Jemuran ditutup");
    }
  }

  // Tampilkan data dengan mengirimkan nilai isIdeal
  tampilkanData(isIdeal);
  
  delay(1000);
}