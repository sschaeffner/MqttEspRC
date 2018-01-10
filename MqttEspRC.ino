#include <PubSubClient.h>
#include <ESP8266WiFi.h>
#include "RCHandler.h"
#include "EepromConfigHandler.h"

EepromConfigHandler conf;
RCHandler rc;
WiFiClientSecure espClient;
PubSubClient mqtt(espClient);
char name[32];
int nameLength;
char mqttServer[64];
char mqttUsername[32];
char mqttPassword[32];

void setup() {
  Serial.begin(115200);
  conf.init();
  rc.init();
  
  char ssid[32];
  char passphrase[64];
  conf.getName(name);
  conf.getWifiSSID(ssid);
  conf.getWifiPassphrase(passphrase);
  conf.getMqttServer(mqttServer);
  conf.getMqttUsername(mqttUsername);
  conf.getMqttPassword(mqttPassword);

  for (int i = 0; i < 32; i++) {
    if (name[i] == 0) {
      nameLength = i;
      break;
    }
  }

  Serial.println();
  Serial.println();
  Serial.println("+ MqttEspRC +");
  Serial.println("=== boot ===");
  Serial.print("name: ");
  Serial.println(name);
  Serial.print("ssid: ");
  Serial.println(ssid);
  Serial.println("============");

  WiFi.begin(ssid, passphrase);
  Serial.println("WiFi: trying to connect...");
}

void loop() {
  conf.loop();
  loop_wifi();
  loop_mqtt();
}

bool firstConn = true;
unsigned long lastDotMillis = 0;
unsigned long currMillis = 0;

void loop_wifi() {
  if (WiFi.status() != WL_CONNECTED) {
    firstConn = true;
    currMillis = millis();
    if (currMillis - lastDotMillis > 500) {
      Serial.println("WiFi: ...");
      lastDotMillis = currMillis;
    }
  } else {
    if (firstConn) {
      firstConn = false;
      Serial.print("WiFi: connected (");
      Serial.print(WiFi.localIP());
      Serial.println(")");

      Serial.println("connecting to MQTT Server...");
      initial_mqtt();
    }
  }
}

void initial_mqtt() {
  mqtt.setServer(mqttServer, 8883);
  mqtt.setCallback(mqttCallback);
}

unsigned long lastDotMillisMqtt = 0;
bool outputMqttStatus = false;

void loop_mqtt() {
  if (WiFi.status() == WL_CONNECTED && !mqtt.connected()) {
    currMillis = millis();
    if (currMillis - lastDotMillisMqtt > 500) {
      Serial.println("MQTT: ...");
      lastDotMillisMqtt = currMillis;
      outputMqttStatus = true;
    } else {
      outputMqttStatus = false;
    }
    // Attempt to connect
    conf.getName(name);
    if (mqtt.connect(name, mqttUsername, mqttPassword)) {
      Serial.println("MQTT: connected");
      
      char nameTopic[34];//32 chars + "/#"
      conf.getName(nameTopic);

      nameTopic[nameLength] = '/';
      nameTopic[nameLength+1] = '#';
      nameTopic[nameLength+2] = 0;
      
      mqtt.subscribe(nameTopic);
    } else {
      if (outputMqttStatus) {
        Serial.print("failed, rc=");
        Serial.println(mqtt.state());
      }
    }
  }
  mqtt.loop();
}

void mqttCallback(char *topic, byte *payload, unsigned int length) {
  char subTopic[10];
  for (int i = 0; i < nameLength + 10; i++) {
    if (topic[i] == 0) {
      if (i > nameLength) {
        subTopic[i - nameLength - 1] = 0;
      }
      break;
    }

    if (i > nameLength) {
      subTopic[i - nameLength - 1] = topic[i];
    }
  }
  
  int num = atoi(subTopic);
  if (num > 0) {
    if (length == 1) {
      if (payload[0] == '1') {
        Serial.print("switching on outlet ");
        Serial.println(num, DEC);
        rc.on(num);
      } else if (payload[0] == '0') {
        rc.off(num);
        Serial.print("switching off outlet ");
        Serial.println(num, DEC);
      }
    }
  }
}

