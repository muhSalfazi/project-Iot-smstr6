#include "SensorDHT.h"
#include "RainSensor.h"
#include "JemuranServo.h"
#include "StatusLED.h"
#include "NetworkManager.h"
#include "MQTTManager.h"

// Pin Definitions
#define RAINPIN_ANALOG 34
#define RAINPIN_DIGITAL 25
#define DHTPIN 27
#define SERVO_PIN 13
#define REDLED 19
#define GREENLED 18

// Global Objects
SensorDHT sensorDHT(DHTPIN, DHT22);
RainSensor rainSensor(RAINPIN_DIGITAL, RAINPIN_ANALOG);
JemuranServo jemuran(SERVO_PIN);
StatusLED statusLED(GREENLED, REDLED);
NetworkManager networkManager;
MQTTManager mqttManager;

// Timing untuk servo, MQTT, dan Serial display
unsigned long lastServoAction = 0;
const unsigned long SERVO_INTERVAL = 100;      // servo check tiap 100 ms
bool needServoUpdate = false;
bool targetServoState = false;                  // true = buka, false = tutup

unsigned long lastDisplay = 0;
const unsigned long DISPLAY_INTERVAL = 3000;   // tampil Serial tiap 3 detik

void setup() {
  Serial.begin(115200);
  initializeComponents();
  mqttManager.setJemuranServo(&jemuran);
  networkManager.connectToWiFi();
  mqttManager.setupMQTT();
}

void loop() {
  mqttManager.checkConnection();

  // Baca sensor
  sensorDHT.bacaData();
  rainSensor.bacaData();

  if (!sensorDHT.isDataValid()) {
    handleSensorError();
    return;
  }

  // Hitung kondisi
  bool isIdeal = calculateIdealConditions();

  // Logika kontrol
  controlLEDs(isIdeal);
  automaticServoControl();        // set targetServoState & needServoUpdate
  handleUserInput();              // bisa mengubah needServoUpdate juga
  mqttManager.publishData(sensorDHT, rainSensor, getKondisiJemur());
  updateServoIfNeeded();          // eksekusi servo non-blocking

  // Tampilkan ke Serial hanya setiap DISPLAY_INTERVAL
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
}

bool calculateIdealConditions() {
  return !rainSensor.getIsRaining() &&
         (sensorDHT.getTemperature() >= 28) &&
         (sensorDHT.getTemperature() <= 35) &&
         (sensorDHT.getHumidity() <= 70) &&
         (sensorDHT.getHeatIndex() < 40) &&
         ((sensorDHT.getTemperature() - sensorDHT.getDewPoint()) > 2);
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

// Set flag untuk servo, tanpa langsung menjalankannya
void automaticServoControl() {
  if (rainSensor.getIsRaining() && jemuran.getStatus()) {
    targetServoState = false;
    needServoUpdate = true;
    Serial.println("WARNING: Hujan terdeteksi! Menutup jemuran...");
  }
  else if (!rainSensor.getIsRaining() && !jemuran.getStatus() && calculateIdealConditions()) {
    targetServoState = true;
    needServoUpdate = true;
    Serial.println("Cuaca ideal, membuka jemuran...");
  }
}

// Eksekusi servo berdasarkan flag, non-blocking
void updateServoIfNeeded() {
  if (needServoUpdate && millis() - lastServoAction >= SERVO_INTERVAL) {
    if (targetServoState && !jemuran.getStatus()) {
      jemuran.buka();
    } 
    else if (!targetServoState && jemuran.getStatus()) {
      jemuran.tutup();
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
    } 
    else if (cmd == 't') {
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
  Serial.println(F("================================"));
}

void handleSensorError() {
  Serial.println(F("Error: gagal baca DHT22!"));
  statusLED.error();
  delay(1000);
}
