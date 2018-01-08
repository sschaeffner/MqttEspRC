#ifndef EEPROM_CONFIG_HANDLER_H
#define EEPROM_CONFIG_HANDLER_H
#include <EEPROM.h>

/*
 * Memory Layout:
 * 000-031: Name (31 chars + termination)
 * 032-063: Wi-Fi SSID (31 chars + termination)
 * 064-127: Wi-Fi passphrase (63 chars + termination)
 * 128-511: free
 */

class EepromConfigHandler {
  public:
    void init();
    void clear();
    void loop();
    int getName(char name[]);
    int getWifiSSID(char ssid[]);
    int getWifiPassphrase(char passphrase[]);
  private:
    int getCharArray(int start, char content[], int length);
    int writeCharArray(int start, char content[], int length, int length_max);
    int setName(char name[], int length);
    int setWifiSSID(char ssid[], int length);
    int setWifiPassphrase(char passphrase[], int length);
    void printPassphrase(char passphrase[], int length);
};
#endif //EEPROM_CONFIG_HANDLER_H
