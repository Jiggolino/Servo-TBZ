#ifndef SERVOTASKS_H
#define SERVOTASKS_H

#include <Arduino.h>
#include "Pingu_Servo.h"

// Servo object
extern Pingu_Servo myServo;

// Physical switch pins
extern const int switchPin0;
extern const int switchPin1;

// Switch states
extern int bit0;
extern int bit1;
extern int bit2;
extern int bit3;

// Servo globals
extern int currentTask;
extern int currentAngle;
extern int lastAngle;
extern int savedPositions[10];
extern int savedCount;

// Function declarations
void writeServo(int angle, const char* msg = nullptr);

void taskA();
void taskB();
void taskC();
void taskD();
void taskE();
void taskF();
void taskG();

void printSwitchStates();
void printHelp();
void handleCommand(String cmd);

#endif
