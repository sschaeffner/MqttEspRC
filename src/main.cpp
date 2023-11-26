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
  digitalWrite(D0, LOW);

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

unsigned long currMillis = 0;

bool wifiFirstConn = true;
unsigned long wifiLastDotMillis = 0;
unsigned long wifiLastConn = 0;
unsigned long wifiLastReboot = 0;

unsigned long wsLastConn = 0;
unsigned long wsLastConnectTry = 0;
unsigned long wsLastConnectCheck = 0;

void loop_wifi() {
  currMillis = millis();

  if (WiFi.status() != WL_CONNECTED) {
    wifiFirstConn = true;    
    if (currMillis - wifiLastDotMillis > 10000) {
      Serial.print("[WiFi] ... (");
      Serial.print(WiFi.status());
      Serial.println(")");
      wifiLastDotMillis = currMillis;
    }
    if (currMillis - wifiLastConn > 30000 && currMillis - wifiLastReboot > 30000) {
      Serial.println("[WiFi] Turning off and on again...");

      WiFi.disconnect(true);
      WiFi.mode(WIFI_OFF);
      WiFi.mode(WIFI_STA);
      WiFi.begin(ssid, passphrase);

      wifiLastReboot = millis();
    }
    if (currMillis - wifiLastConn > 120000) {
      Serial.println("[WiFi] Not connected for 120s. Rebooting...");
      ESP.restart();
    }
  } 

  if (WiFi.status() == WL_CONNECTED) {
    wifiLastConn = currMillis;

    if (wifiFirstConn) {
      wifiFirstConn = false;
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

    if (currMillis - wsLastConn > 15000 && currMillis - wsLastConnectTry > 15000) {
      lastWsConnectTry = currMillis;
      Serial.println("[WS] Connecting...");

      bool connected = wsClient.connectSecure(wsServer, 443, "/ws");
      
      if(connected) {
        Serial.println("[WS] Connected");
        digitalWrite(D0, HIGH);
        wsClient.send("Hello Server");

        wsLastConnectCheck = currMillis;
      } else {
        Serial.println("[WS] Not Connected!");
      }
    }

    if (currMillis - wsLastConnectCheck > 5000) {
      wsLastConnectCheck = currMillis;

      if (wsClient.available(true)) {        
        wsLastConn = currMillis;
      } else {
        Serial.println("[WS] Not connected!");
      }
    } else {
      if (wsClient.available()) {
        wsLastConn = currMillis;
      }
    }

    if (currMillis - wsLastConn > 120000) {
      Serial.println("[WS] Not connected for 120s. Rebooting...");
      ESP.restart();
    }

    wsClient.poll();
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

  if (strlen(message.c_str()) >= 2) {
    if (message.c_str()[0] == 's' && message.c_str()[1] == ':') {
      unsigned long code;
      unsigned int length;
      sscanf(&message.c_str()[2], "%lu,%u", &code, &length);

      Serial.printf("[RC] Sending %lu,%u ...\n", code, length);
      digitalWrite(D0, LOW);
      rc.send(code, length);
      digitalWrite(D0, HIGH);
    }else if (strcmp(message.c_str(), "reboot") == 0) {
      Serial.println("[WS] Got reboot command. Rebooting...");
      ESP.restart();
    }
  }
}
