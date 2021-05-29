#ifndef MAIN_H
#define MAIN_H

void initial_mqtt();

void loop_wifi();
void loop_mqtt();

void mqttCallback(char *topic, byte *payload, unsigned int length);

#endif // MAIN_H