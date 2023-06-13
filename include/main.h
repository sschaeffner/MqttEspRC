#ifndef MAIN_H
#define MAIN_H
#include <Arduino.h>
#include <tiny_websockets/internals/wscrypto/crypto.hpp>
#include <ESP8266WiFi.h>
#include <ArduinoWebsockets.h>
#include "RCHandler.h"
#include "EepromConfigHandler.h"

void setup_ws();

void loop_wifi();
void loop_ws();

void websocketEventCallback(websockets::WebsocketsEvent event, String data);
void websocketMsgCallback(websockets::WebsocketsMessage message);

#endif // MAIN_H