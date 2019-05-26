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

char ssid[32];
char passphrase[64];

void setup() {
  Serial.begin(115200);
  conf.init();
  rc.init();
  
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
  delay(1);
}

bool firstConn = true;
unsigned long lastDotMillis = 0;
unsigned long currMillis = 0;

unsigned long lastConn = 0;
unsigned long lastWifiReboot = 0;

unsigned long lastMqttReconn = 0;

void loop_wifi() {
  if (WiFi.status() != WL_CONNECTED) {
    firstConn = true;
    currMillis = millis();
    if (currMillis - lastDotMillis > 500) {
      Serial.println("WiFi: ...");
      lastDotMillis = currMillis;
    }
    if (currMillis - lastConn > 10000 && currMillis - lastWifiReboot > 10000) {
      Serial.println("Turning WiFi off and on again...");
      
      conf.getWifiSSID(ssid);
      conf.getWifiPassphrase(passphrase);
      
      WiFi.disconnect(true);
      WiFi.mode(WIFI_OFF);
      WiFi.mode(WIFI_STA);
      WiFi.begin(ssid, passphrase);

      lastWifiReboot = millis();
    }
    if (currMillis - lastConn > 60000) {
      Serial.println("Rebooting...");
      ESP.restart();
    }
  } else {
    if (firstConn) {
      firstConn = false;
      Serial.print("WiFi: connected (");
      Serial.print(WiFi.localIP());
      Serial.println(")");

      Serial.println("connecting to MQTT Server...");
      espClient.setInsecure();
      initial_mqtt();
    }
  }
}

void initial_mqtt() {
  conf.getMqttServer(mqttServer);
  
  mqtt.setServer(mqttServer, 8883);
  mqtt.setCallback(mqttCallback);
}

void loop_mqtt() {
  if (WiFi.status() == WL_CONNECTED && !mqtt.connected()) {
    currMillis = millis();

    if (currMillis - lastMqttReconn > 3000) {
      lastMqttReconn = currMillis;
      
      // Attempt to connect
      conf.getName(name);
      conf.getMqttUsername(mqttUsername);
      conf.getMqttPassword(mqttPassword);
      
      Serial.printf("connecting to %s:8883 with %s, %s, %s\n", mqttServer, name, mqttUsername, mqttPassword);
      if (mqtt.connect(name, mqttUsername, mqttPassword)) {
        Serial.println("MQTT: connected");
        
        char nameTopic[34];//32 chars + "/#"
        conf.getName(nameTopic);
  
        nameTopic[nameLength] = '/';
        nameTopic[nameLength+1] = '#';
        nameTopic[nameLength+2] = 0;
  
        Serial.printf("MQTT: subscribing to topic %s'n", nameTopic);
        
        mqtt.subscribe(nameTopic);
      } else {
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
