#include "SensorDHT.h"
#include "RainSensor.h"
#include "JemuranServo.h"
#include "StatusLED.h"
#include "NetworkManager.h"
#include "MQTTManager.h"
#include "Buzzer.h"
#include "LightSensor.h"
#include "RTCManager.h"

// Pin Definitions
#define RAINPIN_ANALOG 34
#define RAINPIN_DIGITAL 25
#define DHTPIN 27
#define SERVO_PIN 13
#define REDLED 19
#define GREENLED 18
#define BUZZER_PIN 26
#define LDR_PIN_ANALOG 35
#define LDR_PIN_DIGITAL 32

// Global Objects
SensorDHT sensorDHT(DHTPIN, DHT22);
RainSensor rainSensor(RAINPIN_DIGITAL, RAINPIN_ANALOG);
JemuranServo jemuran(SERVO_PIN);
StatusLED statusLED(GREENLED, REDLED);
Buzzer buzzer(BUZZER_PIN);
NetworkManager networkManager;
MQTTManager mqttManager;
LightSensor lightSensor(LDR_PIN_ANALOG, LDR_PIN_DIGITAL);
RTCManager rtcManager(networkManager);

// Timing
unsigned long lastServoAction = 0;
const unsigned long SERVO_INTERVAL = 100;
bool needServoUpdate = false;
bool targetServoState = false;
bool closedBecauseRain = false;

unsigned long lastDisplay = 0;
const unsigned long DISPLAY_INTERVAL = 3000;

// Buzzer Control
unsigned long buzzerStartTime = 0;
const unsigned long BUZZER_DURATION = 10000; // 10 detik dalam milidetik
bool isBuzzerActive = false;

void setup() {
  Serial.begin(115200);
  initializeComponents();
  mqttManager.setJemuranServo(&jemuran);
  networkManager.connectToWiFi();
  mqttManager.setupMQTT();
  if (rtcManager.begin()) {
    Serial.print("Waktu saat ini: ");
    Serial.println(rtcManager.getFormattedTime());
  }
}

void loop() {
  mqttManager.checkConnection();

  // Baca sensor
  sensorDHT.bacaData();
  rainSensor.bacaData();

  // Kontrol buzzer
  if (isBuzzerActive) {
    if (millis() - buzzerStartTime < BUZZER_DURATION) {
      buzzer.beep(100); // Bunyi pendek 100ms
      delay(100);       // Jeda 100ms antara beep
    } else {
      isBuzzerActive = false;
      buzzer.stop();
    }
  }

  if (!sensorDHT.isDataValid()) {
    handleSensorError();
    return;
  }

  bool isIdeal = calculateIdealConditions();
  controlLEDs(isIdeal);
  automaticServoControl();
  handleUserInput();
  mqttManager.publishData(sensorDHT, rainSensor, lightSensor, rtcManager, getKondisiJemur());

  updateServoIfNeeded();

  if (millis() - lastDisplay >= DISPLAY_INTERVAL) {
    displayData();
    lastDisplay = millis();
  }
}

void initializeComponents() {
  sensorDHT.begin();
  rainSensor.begin();
  jemuran.begin();
  statusLED.begin();
  buzzer.begin();
  lightSensor.begin();
}

bool calculateIdealConditions() {
  return !rainSensor.getIsRaining() &&
         (sensorDHT.getTemperature() >= 28) &&
         (sensorDHT.getTemperature() <= 35) &&
         (sensorDHT.getHumidity() <= 70) &&
         (sensorDHT.getHeatIndex() < 40) &&
         ((sensorDHT.getTemperature() - sensorDHT.getDewPoint()) > 2) &&
         lightSensor.isCerahAnalog();
}

String getKondisiJemur() {
  if (rainSensor.getIsRaining()) return "HUJAN";

  float suhu = sensorDHT.getTemperature();
  float hum  = sensorDHT.getHumidity();
  float heat = sensorDHT.getHeatIndex();
  float dew  = sensorDHT.getDewPoint();

  if (suhu > 38 || heat > 45) return "BERBAHAYA";
  if (suhu > 33 && hum < 50 && (suhu - dew) > 4) return "SANGAT IDEAL / CEPAT KERING";
  if (suhu >= 28 && suhu <= 33 && hum <= 70 && (suhu - dew) > 2) return "IDEAL";
  return "TIDAK IDEAL";
}

void controlLEDs(bool isIdeal) {
  statusLED.setKondisiIdeal(isIdeal);
  if (!isIdeal) {
    statusLED.kedipMerah(rainSensor.getIsRaining(), rainSensor.getIntensity());
  }
}

void automaticServoControl() {
  // Jika hujan dan jemuran terbuka, tutup jemuran
  if (rainSensor.getIsRaining() && jemuran.getStatus()) {
    targetServoState = false;
    needServoUpdate = true;
    closedBecauseRain = true;
    buzzerStartTime = millis(); // Catat waktu mulai buzzer
    isBuzzerActive = true;      // Aktifkan buzzer
    Serial.println("WARNING: Hujan terdeteksi! Menutup jemuran...");
  } 
  // Jika tidak hujan dan jemuran tertutup karena hujan sebelumnya, langsung buka
  else if (!rainSensor.getIsRaining() && closedBecauseRain && !jemuran.getStatus()) {
    targetServoState = true;
    needServoUpdate = true;
    closedBecauseRain = false;
    isBuzzerActive = false; // Matikan buzzer jika hujan berhenti
    buzzer.stop();          // Pastikan buzzer mati
    Serial.println("Hujan sudah berhenti, membuka jemuran...");
  }
  // Jika kondisi ideal dan jemuran tertutup (bukan karena hujan), buka jemuran
  else if (!rainSensor.getIsRaining() && !jemuran.getStatus() && calculateIdealConditions() && !closedBecauseRain) {
    targetServoState = true;
    needServoUpdate = true;
    Serial.println("Cuaca ideal, membuka jemuran...");
  }
}

void updateServoIfNeeded() {
  if (needServoUpdate && millis() - lastServoAction >= SERVO_INTERVAL) {
    if (targetServoState != jemuran.getStatus()) {  
      if (targetServoState) {
        jemuran.buka();
      } else {
        jemuran.tutup();
      }
      Serial.print("Servo diupdate ke: ");
      Serial.println(targetServoState ? "TERBUKA" : "TERTUTUP");
    }
    needServoUpdate = false;
    lastServoAction = millis();
  }
}

void handleUserInput() {
  if (Serial.available()) {
    char cmd = Serial.read();
    if (cmd == 'b' && !rainSensor.getIsRaining()) {
      targetServoState = true;
      needServoUpdate = true;
      mqttManager.publishAction("buka");
      Serial.println("Perintah manual: buka jemuran");
    } else if (cmd == 't') {
      targetServoState = false;
      needServoUpdate = true;
      mqttManager.publishAction("tutup");
      Serial.println("Perintah manual: tutup jemuran");
    }
  }
}

void displayData() {
  Serial.println(F("\n====== DATA SMART JEMURAN ======"));
  Serial.printf("Suhu       : %.1f °C\n", sensorDHT.getTemperature());
  Serial.printf("Kelembapan : %.1f %%\n", sensorDHT.getHumidity());
  Serial.printf("Heat Index : %.1f °C\n", sensorDHT.getHeatIndex());
  Serial.printf("Dew Point  : %.1f °C\n", sensorDHT.getDewPoint());
  Serial.printf("Status Hujan: %s\n", rainSensor.getRainType());
  Serial.printf("Status Servo: %s\n", jemuran.getStatus() ? "TERBUKA" : "TERTUTUP");
  Serial.printf("Kondisi     : %s\n", getKondisiJemur().c_str());
  Serial.printf("LDR Analog : %d (%s)\n", lightSensor.bacaAnalog(), lightSensor.isCerahAnalog() ? "Cerah" : "Gelap");
  Serial.printf("LDR Digital: %s\n", lightSensor.bacaDigital() ? "Cerah" : "Gelap");
  Serial.printf("Waktu      : %s\n", rtcManager.getFormattedTime().c_str());

  Serial.println(F("================================"));
}

void handleSensorError() {
  Serial.println(F("Error: gagal baca DHT22!"));
  statusLED.error();
  delay(1000);
}