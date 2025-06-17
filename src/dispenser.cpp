#include "dispenser.h"
#include "config.h"
#include "connection.h"
#include "stepper_motor.h"
#include "IR_sensor.h"
#include <Arduino.h>
#include <ArduinoJson.h>


/* CMD
#define CMD {"flag" : 0, "pos" : [0, 0, 0, 0], "count" : [0, 0, 0, 0]}
flag 0 = DISPENSE, 1 = OPEN, 2 = CLOSE, 3 = CW 1 STEP, 4 = CCW 1 STEP
pos = 0 disable 1 enable for each position in order LEFT_BOTTOM, LEFT_TOP, RIGHT_BOTTOM, RIGHT_TOP
count = number for each position in order LEFT_BOTTOM, LEFT_TOP, RIGHT_BOTTOM, RIGHT_TOP
*/
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

bool isBusy()
{
    return hasTask;
}

void queueTask(Position pos, int count)
{
    currentTask.pos = pos;
    currentTask.count = count;
    hasTask = true;
    publishStatus(1, "RUNNING");
}

void loopDispenser()
{
    openOne(currentTask.pos);
    delay(MOTOR_INTERVAL);
    closeOne(currentTask.pos);
    delay(RESET_INTERVAL);
    resetMotorPin();

    currentTask.count--;

    if (currentTask.count <= 0)
    {
        hasTask = false;
        afterDrop();
        publishStatus(1, "DONE");
        Serial.println("Task completed");
    }
}