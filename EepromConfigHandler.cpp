#include "EepromConfigHandler.h"
#include <HardwareSerial.h>

void EepromConfigHandler::init() {
  EEPROM.begin(512);
}


char inBuf;
char mode;
int pos;
char buffer[64];
void EepromConfigHandler::loop() {
  if (pos == 64) {
    Serial.println("buffer full. resetting input.");
    pos = 0;
  }
  
  if (Serial.available() <= 0) {
    return;
  }
  
  inBuf = Serial.read();
  
  switch (mode) {
    case 0://general menu
        switch (inBuf) {
          case 'h':
            Serial.println("=== help ===");
            Serial.println("commands available: ");
            Serial.println("* h - print this help");
            Serial.println("* l - list all settings");
            Serial.println("* n - set name of device");
            Serial.println("* s - set ssid of wifi to connect to");
            Serial.println("* p - set passphrase of wifi to connect to");
            Serial.println("* c - clear configuration");
            Serial.println("============");
            break;
          case 'n':
            mode = 'n';
            pos = 0;
            Serial.print("name: ");
            while (Serial.available() > 0) Serial.read();
            break;
          case 's':
            mode = 's';
            pos = 0;
            Serial.print("ssid: ");
            while (Serial.available() > 0) Serial.read();
            break;
          case 'p':
            mode = 'p';
            pos = 0;
            Serial.print("passphrase: ");
            while (Serial.available() > 0) Serial.read();
            break;
          case 'l':
            char nameBuf[32];
            char ssidBuf[32];
            char passphraseBuf[64];
            getName(nameBuf);
            getWifiSSID(ssidBuf);
            getWifiPassphrase(passphraseBuf);
            Serial.println("=== config ===");
            Serial.print("name: ");
            Serial.println(nameBuf);
            Serial.print("ssid: ");
            Serial.println(ssidBuf);
            Serial.print("passphrase: ");
            printPassphrase(passphraseBuf, 32);
            Serial.println();
            Serial.println("==============");
            break;
          case 'c':
            Serial.println("Clear configuration: sure? y/n");
            while (Serial.available() > 0) Serial.read();
            mode = 'c';
            break;
          case '\n':
          case '\r':
            break;
          default:
            mode = 0;
            Serial.print("unknown command: ");
            Serial.println(inBuf);
            break;
        }
      break;
    case 'n'://name
      if (pos == 0 && (inBuf == '\n' || inBuf == '\r')) {
        //do not increase pos
      } else if (pos > 0 && (inBuf == '\n' || inBuf == '\r')) {
        buffer[pos] = 0;//terminate string
        setName(buffer, pos);
        mode = 0;
        Serial.println();
        Serial.println("new name set");
      } else {
        Serial.print(inBuf);
        buffer[pos++] = inBuf;
      }
      break;
    case 's'://ssid
      if (pos == 0 && (inBuf == '\n' || inBuf == '\r')) {
        //do not increase pos
      } else if (pos > 0 && (inBuf == '\n' || inBuf == '\r')) {
        buffer[pos] = 0;//terminate string
        setWifiSSID(buffer, pos);
        mode = 0;
        Serial.println();
        Serial.println("new ssid set");
      } else {
        Serial.print(inBuf);
        buffer[pos++] = inBuf;
      }
      break;
    case 'p'://passphrase
      if (pos == 0 && (inBuf == '\n' || inBuf == '\r')) {
        //do not increase pos
      } else if (pos > 0 && (inBuf == '\n' || inBuf == '\r')) {
        buffer[pos] = 0;//terminate string
        setWifiPassphrase(buffer, pos);
        mode = 0;
        Serial.println();
        Serial.println("new passphrase set");
      } else {
        Serial.print(inBuf);
        buffer[pos++] = inBuf;
      }
      break;
    case 'c'://clear
      if (inBuf == 'y') {
        Serial.println("clearing config...");
        clear();
        Serial.println("...done");
        mode = 0;
      } else if (inBuf == 'n') {
        Serial.println("aborted");
        mode = 0;
      }
      break;
    default:
      mode = 0;
      break;
  }
  
}

void EepromConfigHandler::clear() {
  for (int i = 0; i < 512; i++) {
    EEPROM.write(i, 0);
  }
  EEPROM.commit();
}

int EepromConfigHandler::getName(char name[]) {
  return getCharArray(0, name, 32);
}

int EepromConfigHandler::setName(char name[], int length) {
  return writeCharArray(0, name, length, 32);
}


int EepromConfigHandler::getWifiSSID(char ssid[]) {
  return getCharArray(32, ssid, 32);
}

int EepromConfigHandler::setWifiSSID(char ssid[], int length) {
  return writeCharArray(32, ssid, length, 32);
}

int EepromConfigHandler::getWifiPassphrase(char passphrase[]){
  return getCharArray(64, passphrase, 64);
}

int EepromConfigHandler::setWifiPassphrase(char passphrase[], int length) {
  return writeCharArray(64, passphrase, length, 64);
}

/*
 * Writes a character array to EEPROM and forces null-termination.
 * 
 * start     - position of array in EEPROM
 * content   - character array to write to the EEPROM
 * length    - length of content array
 * lengt_max - max allowed length for this array
 */
int EepromConfigHandler::writeCharArray(int start, char content[], int length, int length_max) {
  for (int i = 0; i < length_max; i++) {
    if (i < length && i < length_max - 1) {
      EEPROM.write(start + i, content[i]);
    } else {
      EEPROM.write(start + i, 0);
    }
  }
  EEPROM.commit();
  return 0;
}

/*
 * Reads a character array from EEPROM.
 * 
 * start   - position of array in EEPROM
 * content - character array to write into
 * length  - length of the content array
 */
int EepromConfigHandler::getCharArray(int start, char content[], int length) {
  int p = -1;
  for (int i = 0; i < length; i++) {
    p = start + i;
    if (p > 511) return -1;
    content[i] = EEPROM.read(p);
  }

  return 0;
}

void EepromConfigHandler::printPassphrase(char passphrase[], int length) {
  for (int i = 0; i < length; i++) {
    if (passphrase[i] == 0) {
      return;
    } else {
      Serial.print('*');
    }
  }
}

