#include "dispenser.h"
#include "config.h"
#include "connection.h"
#include "stepper_motor.h"
#include "IR_sensor.h"
#include <Arduino.h>
#include <ArduinoJson.h>

struct Task
{
    Position pos;
    int count;
};

static Task currentTask;
static bool hasTask = false;

void initDispenser()
{
    initStepperMotor();
    initIR();
    hasTask = false;
}

bool isBusy(){
    return hasTask;
}

void queueTask(Position pos, int count)
{
    currentTask.pos = pos;
    currentTask.count = count;
    hasTask = true;
}

void loopDispenser()
{
    dropOne(currentTask.pos);
    currentTask.count--;

    if (currentTask.count <= 0)
    {
        hasTask = false;
        afterDrop();
        publishStatus(0, "Task completed");
        Serial.println("Task completed");
    }
    delay(100);
}