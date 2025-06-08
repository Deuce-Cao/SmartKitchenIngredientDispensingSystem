#ifndef DISPENSER_H
#define DISPENSER_H

void initDispenser();
bool checkOccupancy();
bool openGate();
bool dispenseUnits(int count);
bool handleCommand(int count);

#endif