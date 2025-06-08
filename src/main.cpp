#include <Arduino.h>
#include "connection.h"

void setup() {
  Serial.begin(115200);
  connectToWiFi();
  connectToMQTT();
}

void loop() {
  loopMQTT();
  delay(1000); // Adjust the delay as needed
}