#ifndef DISPENSER_COPY_H
#define DISPENSER_COPY_H
#include "config.h"

void initDispenser();
void loopDispenser();
bool isBusy();
void queueTask(int flag, bool pos[], int count[]);

#endif