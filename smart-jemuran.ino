#include "SensorDHT.h"
#include "RainSensor.h"
#include "JemuranServo.h"
#include "StatusLED.h"
#include "NetworkManager.h"
#include "MQTTManager.h"

// Pin Definitions
#define DHTPIN 17
#define RAINPIN_DIGITAL 15
#define RAINPIN_ANALOG 35
#define REDLED 18
#define GREENLED 19
#define SERVO_PIN 21

// Global Objects
SensorDHT sensorDHT(DHTPIN, DHT11);
RainSensor rainSensor(RAINPIN_DIGITAL, RAINPIN_ANALOG);
JemuranServo jemuran(SERVO_PIN);
StatusLED statusLED(GREENLED, REDLED);
NetworkManager networkManager;
MQTTManager mqttManager;

void setup() {
  Serial.begin(115200);
  initializeComponents();
  mqttManager.setJemuranServo(&jemuran);
  networkManager.connectToWiFi();
  mqttManager.setupMQTT();
  mqttManager.checkConnection();
}

void loop() {
  mqttManager.checkConnection();

  // Sensor operations
  sensorDHT.bacaData();
  rainSensor.bacaData();

  if (!sensorDHT.isDataValid()) {
    handleSensorError();
    return;
  }

  bool isIdeal = calculateIdealConditions();

  // Control systems
  controlLEDs(isIdeal);
  automaticServoControl();
  handleUserInput();

  // Data display and transmission
  displayData(isIdeal);
  mqttManager.publishData(sensorDHT, rainSensor, isIdeal);

  delay(1000);
}

void initializeComponents() {
  sensorDHT.begin();
  rainSensor.begin();
  jemuran.begin();
  statusLED.begin();
}

bool calculateIdealConditions() {
  return !rainSensor.getIsRaining() && (sensorDHT.getTemperature() >= 28) && (sensorDHT.getTemperature() <= 35) && (sensorDHT.getHumidity() <= 70) && (sensorDHT.getHeatIndex() < 40) && ((sensorDHT.getTemperature() - sensorDHT.getDewPoint()) > 2);
}

void controlLEDs(bool isIdeal) {
  statusLED.setKondisiIdeal(isIdeal);
  if (!isIdeal) {
    statusLED.kedipMerah(rainSensor.getIsRaining(), rainSensor.getIntensity());
  }
}

void automaticServoControl() {
  if (rainSensor.getIsRaining() && jemuran.getStatus()) {
    jemuran.tutup();
    Serial.println("WARNING: Hujan terdeteksi! Jemuran ditutup otomatis");
  } else if (!rainSensor.getIsRaining() && !jemuran.getStatus() && calculateIdealConditions()) {
    jemuran.buka();
    Serial.println("Cuaca ideal, jemuran dibuka otomatis");
  }
}


void handleUserInput() {
  if (Serial.available()) {
    char input = Serial.read();
    if (input == 'b' && !rainSensor.getIsRaining()) {
      jemuran.buka();
      Serial.println("Jemuran dibuka manual");
      mqttManager.publishAction("buka");
    } else if (input == 't') {
      jemuran.tutup();
      Serial.println("Jemuran ditutup manual");
      mqttManager.publishAction("tutup");
    }
  }
}

void displayData(bool isIdeal) {
  Serial.println("\n========== DATA ==========");
  Serial.print("Suhu: ");
  Serial.print(sensorDHT.getTemperature(), 1);
  Serial.println(" °C");
  Serial.print("Kelembapan: ");
  Serial.print(sensorDHT.getHumidity(), 1);
  Serial.println(" %");
  Serial.print("Indeks Panas: ");
  Serial.print(sensorDHT.getHeatIndex(), 1);
  Serial.println(" °C");
  Serial.print("Titik Embun: ");
  Serial.print(sensorDHT.getDewPoint(), 1);
  Serial.println(" °C");
  Serial.print("Status Hujan: ");
  Serial.println(rainSensor.getRainType());
  Serial.print("Status Jemuran: ");
  Serial.println(jemuran.getStatus() ? "TERBUKA" : "TERTUTUP");
  Serial.print("Kondisi Jemur: ");
  Serial.println(isIdeal ? "IDEAL" : "TIDAK IDEAL");
  Serial.println("========================");
}

void handleSensorError() {
  Serial.println("Gagal baca DHT11!");
  statusLED.error();
  delay(1000);
}
