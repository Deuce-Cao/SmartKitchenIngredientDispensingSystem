#include <Arduino.h>
#include "connection.h"
#include "dispenser.h"

void setup() {
  Serial.begin(115200);
  connectToWiFi();
  connectToMQTT();
  initDispenser();
  publishStatus("initialized");
}

void loop() {
  loopMQTT();
  delay(1000); // Adjust the delay as needed
}