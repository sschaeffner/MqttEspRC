#include <PubSubClient.h>
#include <ESP8266WiFi.h>
#include "RCHandler.h"
#include "EepromConfigHandler.h"

const char* mqtt_server = "192.168.0.15";

EepromConfigHandler conf;
RCHandler rc;
WiFiClient espClient;
PubSubClient mqtt(espClient);
char name[32];
int nameLength;

void setup() {
  Serial.begin(115200);
  conf.init();
  rc.init();
  
  char ssid[32];
  char passphrase[64];
  conf.getName(name);
  conf.getWifiSSID(ssid);
  conf.getWifiPassphrase(passphrase);

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
  mqtt.setServer(mqtt_server, 1883);
  mqtt.setCallback(mqttCallback);
}

void loop_mqtt() {
  if (WiFi.status() == WL_CONNECTED && !mqtt.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    conf.getName(name);
    if (mqtt.connect(name)) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      //mqtt.publish("outTopic", "hello world");
      // ... and resubscribe
      char nameTopic[34];//32 chars + "/#"
      conf.getName(nameTopic);

      nameTopic[nameLength] = '/';
      nameTopic[nameLength+1] = '#';
      nameTopic[nameLength+2] = 0;
      
      mqtt.subscribe(nameTopic);
    } else {
      Serial.print("failed, rc=");
      Serial.print(mqtt.state());
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
      Serial.println();
      break;
    }

    if (i > nameLength) {
      subTopic[i - nameLength - 1] = topic[i];
    }
  }
  
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();

  Serial.print("subTopic: ");
  Serial.println(subTopic);

  int num = atoi(subTopic);
  if (num > 0) {
    Serial.println("subTopic is number");
    Serial.print("num: ");
    Serial.println(num, DEC);

    if (length == 1) {
      if (payload[0] == '1') {
        Serial.println("switching on...");
        rc.on(num);
      } else if (payload[0] == '0') {
        rc.off(num);
        Serial.println("switching off...");
      }
    }
  }
}

