#ifndef DISPENSER_H
#define DISPENSER_H
#include "config.h"

void initDispenser();
void loopDispenser();
bool isBusy();
void queueTask(Position pos, int count);

#endif