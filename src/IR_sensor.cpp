#include <Arduino.h>
#include "IR_sensor.h"
#include "config.h"
#include "connection.h"

const int sensorPin = SENSOR_PIN_OUT;
const int sensorThreshold = SENSOR_THRESHOLD; enum HolderState {
    EMPTY,
    READY,
    OCCUPIED,
    REMOVED,
};

static HolderState holderState = EMPTY;

void initIR()
{
    pinMode((sensorPin), INPUT);
}

bool isDetected()
{
    float x = analogRead(sensorPin);
    return x < sensorThreshold;
}

void updateHolder()
{
    switch (holderState)
    {
    case EMPTY:
        if (isDetected())
            holderState = READY;
        break;
    case READY:
        if (!isDetected())
            holderState = EMPTY;
        break;
    case OCCUPIED:
        if (!isDetected())
            holderState = REMOVED;
        break;
    case REMOVED:
        if (isDetected())
            holderState = READY;
        break;
    }
    delay(100);
}

void afterDrop()
{
    holderState = OCCUPIED;
}

bool isAvailable(){
    return holderState == READY;
}