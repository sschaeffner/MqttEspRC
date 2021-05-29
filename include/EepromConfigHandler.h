#ifndef EEPROM_CONFIG_HANDLER_H
#define EEPROM_CONFIG_HANDLER_H
#include <EEPROM.h>

/*
 * Memory Layout:
 * 000-031: Name (31 chars + termination)
 * 032-063: Wi-Fi SSID (31 chars + termination)
 * 064-127: Wi-Fi passphrase (63 chars + termination)
 * 128-191: MQTT Server (63 chars + termination)
 * 192-223: MQTT Username (31 chars + termination)
 * 224-255: MQTT Password (31 chars + termination)
 * 256-511: free
 */

class EepromConfigHandler {
  public:
    void init();
    void clear();
    void loop();
    int getName(char name[]);
    int getWifiSSID(char ssid[]);
    int getWifiPassphrase(char passphrase[]);
    int getMqttServer(char server[]);
    int getMqttUsername(char username[]);
    int getMqttPassword(char password[]);
  private:
    int getCharArray(int start, char content[], int length);
    int writeCharArray(int start, char content[], int length, int length_max);
    int setName(char name[], int length);
    int setWifiSSID(char ssid[], int length);
    int setWifiPassphrase(char passphrase[], int length);
    void printPassphrase(char passphrase[], int length);
    int setMqttServer(char userver[], int length);
    int setMqttUsername(char username[], int length);
    int setMqttPassword(char password[], int length);
};
#endif //EEPROM_CONFIG_HANDLER_H
