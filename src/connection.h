void connectToWiFi();

void connectToMQTT();
void publishStatus(String status);
void mqttCallback(char *topic, byte *payload, unsigned int length);
void loopMQTT();