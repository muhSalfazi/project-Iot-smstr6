#ifndef MQTTMANAGER_H
#define MQTTMANAGER_H

#include <PubSubClient.h>
#include <WiFi.h>
#include <ArduinoJson.h>
#include "SensorDHT.h"
#include "RainSensor.h"
#include "JemuranServo.h"

class MQTTManager {
  private:
    WiFiClient espClient;
    PubSubClient mqttClient;
    JemuranServo* jemuranServo;  // Pointer to JemuranServo
    
    const char* mqtt_server = "192.168.244.254"; // Local broker IP (removed space)
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
          if (jemuranServo != nullptr) {
            jemuranServo->buka();
          }
        } else if (strcmp(message, "tutup") == 0) {
          if (jemuranServo != nullptr) {
            jemuranServo->tutup();
          }else {
          Serial.println("Perintah tidak dikenali");
        }
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
    
    void publishData(SensorDHT& dht, RainSensor& rain, bool isIdeal) {
      StaticJsonDocument<200> doc;
      doc["suhu"] = dht.getTemperature();
      doc["kelembapan"] = dht.getHumidity();
      doc["heat_index"] = dht.getHeatIndex();
      doc["dew_point"] = dht.getDewPoint();
      doc["hujan"] = rain.getRainType();
      if(jemuranServo != nullptr) {
        doc["status_jemuran"] = jemuranServo->getStatus() ? "TERBUKA" : "TERTUTUP";
      }
      doc["kondisi_ideal"] = isIdeal ? "IDEAL" : "TIDAK IDEAL";

      char payload[256];
      serializeJson(doc, payload);
      mqttClient.publish(topic_publish, payload);
    }
    
    void publishAction(const char* action) {
      mqttClient.publish(topic_publish, action);
    }
    unsigned long startAttempt = millis();
    void reconnect() {
     while (!mqttClient.connected() && millis() - startAttempt < 15000) {
        Serial.print("Coba konek MQTT ke ");
        Serial.print(mqtt_server);
        Serial.print(":");
        Serial.println(mqtt_port);

        // Uji koneksi TCP dulu
        if (!espClient.connect(mqtt_server, mqtt_port)) {
          Serial.println("❌ Tidak bisa connect TCP ke broker MQTT.");
          delay(5000);
          continue;
        }
        espClient.stop(); // tutup tes koneksi
        
        if (mqttClient.connect("ESP32Client")) {
          Serial.println("✅ MQTT connected");
          mqttClient.subscribe(topic_subscribe);
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