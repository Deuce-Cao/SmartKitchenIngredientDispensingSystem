#include "dispenser.h"
#include "config.h"
#include "connection.h"
#include <Arduino.h>
#include <ArduinoJson.h>

int motorPin = MOTOR_PIN;
int sensorPin = SENSOR_PIN;

void initDispenser()
{
    pinMode(motorPin, OUTPUT);
    pinMode(sensorPin, INPUT);
    // Initialize motor (servo, stepper, etc.)
}

bool checkOccupancy()
{
    int val = digitalRead(sensorPin); // or analogRead if applicable
    return (val == HIGH);             // true means blocked
}

bool openGate()
{
    // Send signal to motor to open & close gate once
    // Motor code to be filled based on motor type
    Serial.println("Gate opened once.");
    return true; // or false if failed
}

bool dispenseUnits(int count)
{
    for (int i = 0; i < count; i++)
    {
        bool success = openGate();
        if (!success)
        {
            Serial.println("Error during gate open.");
            return false;
        }
        delay(500); // brief pause between units (tune as needed)
    }
    publishStatus(String(count));
    return true;
}

bool handleCommand(int count)
{
    Serial.printf("Handling dispense command: %d unit(s)\n", count);

    if (checkOccupancy())
    {   
        publishStatus("Blocked");
        Serial.println("Dispense blocked. Area occupied.");
        return false;
    }

    bool result = dispenseUnits(count);
    return result;
}