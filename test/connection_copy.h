#include <Arduino.h>
#ifndef CONNECTION_COPY_H
#define CONNECTION_COPY_H

void connectToWiFi();

void connectToMQTT();
void publishStatus(bool att, String status);
void mqttCallback(char *topic, byte *payload, unsigned int length);
void reconnectMQTT();
void loopMQTT();
#endif