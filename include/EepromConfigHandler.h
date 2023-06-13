#ifndef EEPROM_CONFIG_HANDLER_H
#define EEPROM_CONFIG_HANDLER_H
#include <EEPROM.h>

/*
 * Memory Layout:
 * 000-031: free (31 chars + termination)
 * 032-063: Wi-Fi SSID (31 chars + termination)
 * 064-127: Wi-Fi passphrase (63 chars + termination)
 * 128-191: WS Server (63 chars + termination)
 * 192-223: WS Username (31 chars + termination)
 * 224-255: WS Password (31 chars + termination)
 * 256-511: free
 */

class EepromConfigHandler {
  public:
    void init();
    void clear();
    void loop();
    int getWifiSSID(char ssid[]);
    int getWifiPassphrase(char passphrase[]);
    int getWsServer(char server[]);
    int getWsUsername(char username[]);
    int getWsPassword(char password[]);
  private:
    int getCharArray(int start, char content[], int length);
    int writeCharArray(int start, char content[], int length, int length_max);
    int setWifiSSID(char ssid[], int length);
    int setWifiPassphrase(char passphrase[], int length);
    void printPassphrase(char passphrase[], int length);
    int setWsServer(char userver[], int length);
    int setMqttUsername(char username[], int length);
    int setWsPassword(char password[], int length);
};
#endif //EEPROM_CONFIG_HANDLER_H
