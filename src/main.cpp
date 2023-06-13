#include "main.h"

EepromConfigHandler conf;
RCHandler rc;
websockets::WebsocketsClient wsClient;

char wsServer[64];
char wsUsername[32];
char wsPassword[32];

char ssid[32];
char passphrase[64];

void setup() {
  pinMode(D0, OUTPUT);
  digitalWrite(D0, HIGH);

  Serial.begin(115200);
  conf.init();
  rc.init();
  
  conf.getWifiSSID(ssid);
  conf.getWifiPassphrase(passphrase);
  conf.getWsServer(wsServer);
  conf.getWsUsername(wsUsername);
  conf.getWsPassword(wsPassword);

  Serial.println();
  Serial.println();
  Serial.println("+ WsEspRC +");
  Serial.println("=== boot ===");
  Serial.print("ssid: ");
  Serial.println(ssid);
  Serial.println("============");

  setup_ws();

  WiFi.begin(ssid, passphrase);
  Serial.println("WiFi: trying to connect...");
}

void loop() {
  conf.loop();
  loop_wifi();
  loop_ws();
  delay(1);
  yield();
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
    if (currMillis - lastDotMillis > 10000) {
      Serial.print("[WiFi] ... (");
      Serial.print(WiFi.status());
      Serial.println(")");
      lastDotMillis = currMillis;
    }
    if (currMillis - lastConn > 30000 && currMillis - lastWifiReboot > 30000) {
      Serial.println("[WiFi] Turning off and on again...");
      
      conf.getWifiSSID(ssid);
      conf.getWifiPassphrase(passphrase);
      
      WiFi.disconnect(true);
      WiFi.mode(WIFI_OFF);
      WiFi.mode(WIFI_STA);
      WiFi.begin(ssid, passphrase);

      lastWifiReboot = millis();
    }
    if (currMillis - lastConn > 120000) {
      Serial.println("Rebooting...");
      ESP.restart();
    }
  } else {
    if (firstConn) {
      firstConn = false;
      Serial.print("[WiFi] connected (");
      Serial.print(WiFi.localIP());
      Serial.println(")");
    }
  }
}

void setup_ws() {
  
  wsClient.setInsecure();

  String auth = wsUsername;
  auth += ":";
  auth += wsPassword;
  websockets::WSString base64 = websockets::crypto::base64Encode((uint8_t *)auth.c_str(), auth.length());
  String authstr = "Basic ";
  authstr += websockets::internals::fromInternalString(base64);
  wsClient.addHeader("Authorization", authstr);

  wsClient.onEvent(websocketEventCallback);
  wsClient.onMessage(websocketMsgCallback);
}

unsigned long lastWsAvailable = 0;
unsigned long lastWsConnectTry = 0;

void loop_ws() {
  if (WiFi.status() == WL_CONNECTED) {
    currMillis = millis();

    if (currMillis - lastWsAvailable > 15000 && currMillis - lastWsConnectTry > 15000) {
      lastWsConnectTry = currMillis;
      Serial.println("[WS] Connecting...");

      bool connected = wsClient.connectSecure(wsServer, 443, "/ws");
      
      if(connected) {
        Serial.println("[WS] Connected");
        wsClient.send("Hello Server");
      } else {
        Serial.println("[WS] Not Connected!");
      }
    }

    if(wsClient.available()) {
      wsClient.poll();
      lastWsAvailable = currMillis;
    }
  }
}

void websocketEventCallback(websockets::WebsocketsEvent event, String data) {
  websockets::CloseReason reason = wsClient.getCloseReason();

  switch(event) {
    case websockets::WebsocketsEvent::ConnectionOpened:
      Serial.println("[WS] ConnectionOpened");
      break;
    case websockets::WebsocketsEvent::ConnectionClosed:
      Serial.printf("[WS] ConnectionClosed reason: %d\n", reason);
      break;       
    case websockets::WebsocketsEvent::GotPing:
      Serial.println("[WS] GotPing");
      digitalWrite(D0, LOW);
      delay(10);
      digitalWrite(D0, HIGH);
      break;
    case websockets::WebsocketsEvent::GotPong:
      Serial.println("[WS] GotPong");
      break;
    default:
      Serial.println("[WS] other event");
      break;
  }
}

void websocketMsgCallback(websockets::WebsocketsMessage message) {
  Serial.print("[WS] Message: ");
  Serial.println(message.c_str());
}
