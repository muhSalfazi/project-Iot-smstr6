#ifndef MQTTMANAGER_H
#define MQTTMANAGER_H

#include <PubSubClient.h>
#include <WiFi.h>
#include <ArduinoJson.h>
#include "SensorDHT.h"
#include "RainSensor.h"
#include "JemuranServo.h"

// Variabel global dari main.ino
extern bool needServoUpdate;
extern bool targetServoState;

class MQTTManager {
  private:
    WiFiClient espClient;
    PubSubClient mqttClient;
    JemuranServo* jemuranServo;

    const char* mqtt_server = "192.168.244.254"; // IP broker lokal
    const int mqtt_port = 1884;
    const char* topic_publish = "jemuran/data";
    const char* topic_subscribe = "jemuran/control";

    void callback(char* topic, byte* payload, unsigned int length) {
      Serial.print("Pesan tiba [");
      Serial.print(topic);
      Serial.print("] ");

      char message[length + 1];
      memcpy(message, payload, length);
      message[length] = '\0';
      Serial.println(message);

      if (strcmp(topic, topic_subscribe) == 0) {
        if (strcmp(message, "buka") == 0) {
          Serial.println("MQTT: membuka jemuran...");
          if (jemuranServo != nullptr) {
            targetServoState = true;
            needServoUpdate = true;
          } else {
            Serial.println("jemuranServo tidak diinisialisasi!");
          }
        } else if (strcmp(message, "tutup") == 0) {
          Serial.println("MQTT: menutup jemuran...");
          if (jemuranServo != nullptr) {
            targetServoState = false;
            needServoUpdate = true;
          } else {
            Serial.println("jemuranServo tidak diinisialisasi!");
          }
        } else {
          Serial.println("Perintah tidak dikenali");
        }
      }
    }

  public:
    MQTTManager() : mqttClient(espClient), jemuranServo(nullptr) {}

    void setJemuranServo(JemuranServo* servo) {
      jemuranServo = servo;
    }

    void setupMQTT() {
      Serial.print("Set MQTT broker: ");
      Serial.print(mqtt_server);
      Serial.print(":");
      Serial.println(mqtt_port);

      mqttClient.setServer(mqtt_server, mqtt_port);
      mqttClient.setCallback([this](char* topic, byte* payload, unsigned int length) {
        this->callback(topic, payload, length);
      });
    }

    void checkConnection() {
      if (!mqttClient.connected()) {
        reconnect();
      }
      mqttClient.loop();
    }

    void publishData(SensorDHT& dht, RainSensor& rain, const String& kondisiJemur) {
      StaticJsonDocument<200> doc;
      doc["suhu"] = dht.getTemperature();
      doc["kelembapan"] = dht.getHumidity();
      doc["heat_index"] = dht.getHeatIndex();
      doc["dew_point"] = dht.getDewPoint();
      doc["hujan"] = rain.getRainType();
      doc["kondisi"] = kondisiJemur;

      if (jemuranServo != nullptr) {
        doc["status_jemuran"] = jemuranServo->getStatus() ? "TERBUKA" : "TERTUTUP";
      }

      char payload[256];
      serializeJson(doc, payload);
      mqttClient.publish(topic_publish, payload);
    }

    void publishAction(const char* action) {
      mqttClient.publish(topic_publish, action);
    }

    void reconnect() {
      static unsigned long startAttempt = millis();
      while (!mqttClient.connected() && millis() - startAttempt < 15000) {
        Serial.print("Coba konek MQTT ke ");
        Serial.print(mqtt_server);
        Serial.print(":");
        Serial.println(mqtt_port);

        if (mqttClient.connect("ESP32Client")) {
          Serial.println("✅ MQTT connected");
          mqttClient.subscribe(topic_subscribe);
          startAttempt = millis(); // reset timer
          break;
        } else {
          Serial.print("❌ MQTT failed, rc=");
          Serial.print(mqttClient.state());
          Serial.println(" coba lagi 5 detik...");
          delay(5000);
        }
      }
    }
};

#endif
