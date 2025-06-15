#include <Arduino.h>
#include "connection.h"
#include "dispenser.h"
#include "IR_sensor.h"

static bool isReported = false;

void reportHolder()
{
  if (!isAvailable() && !isReported)
  {
    Serial.println("Holder unavailable");
    publishStatus(1, "Holder unavailable");
    isReported = true;
  }
  if (isAvailable() && isReported)
  {
    Serial.println("Holder available");
    publishStatus(0, "Holder available");
    isReported = false;
  }
  delay(500);
}

void setup()
{
  Serial.begin(115200);
  connectToWiFi();
  connectToMQTT();
  initDispenser();
  publishStatus(0, "initialized");
}

void loop()
{
  loopMQTT();
  updateHolder();
  reportHolder();
  if (isAvailable() && isBusy())
  {
    loopDispenser();
  }
}