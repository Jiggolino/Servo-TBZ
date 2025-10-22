#include "ServoTasks.h"

// --- Global Objects---
Pingu_Servo myServo;

const int switchPin0 = 18;
const int switchPin1 = 19;

int bit0 = HIGH;
int bit1 = HIGH;
int bit2 = HIGH;
int bit3 = HIGH;

int currentTask = 0;
int currentAngle = 90;
int lastAngle = -1;
int savedPositions[10];
int savedCount = 0;

// --- Helper: safe write with change detection ---
void writeServo(int angle, const char* msg) {
    angle = constrain(angle, 0, 180);
    if (angle != lastAngle) {
        myServo.write(angle);
        lastAngle = angle;
        if (msg) {
            Serial.printf("%s %d°\n", msg, angle);
        } else {
            Serial.printf("Servo -> %d°\n", angle);
        }
    }
}

// --- Task implementations ---
void taskA() {
    writeServo(90, "Task A: Servo");
}

void taskB() {
    if (bit0 == LOW) writeServo(0, "Task B: Servo");
    else writeServo(180, "Task B: Servo");
}

void taskC() {
    int positions[10] = {0,20,40,60,80,100,120,140,160,180}; //10 positions
    for (int i=0; i<10; i++) {
        writeServo(positions[i], "Task C: Servo");
        delay(1000);
    }
}

void taskD() {
    if (bit0 == LOW && currentAngle > 0) currentAngle -= 10;
    if (bit1 == LOW && currentAngle < 180) currentAngle += 10;
    writeServo(currentAngle, "Task D: Servo");
    delay(200);
}

void taskE() {
    static unsigned long pressTime = 0;
    static int speed = 200;
    if (bit0 == LOW || bit1 == LOW) {
        if (pressTime == 0) pressTime = millis();
        unsigned long held = millis() - pressTime;
        if (held > 1000 && speed > 20) speed -= 10;
        if (bit0 == LOW && currentAngle > 0) currentAngle -= 5;
        if (bit1 == LOW && currentAngle < 180) currentAngle += 5;
        writeServo(currentAngle, "Task E: Servo");
        delay(speed);
    } else {
        pressTime = 0;
        speed = 200;
    }
}

void taskF() {
    taskE();
    if (bit2 == LOW && savedCount < 10) {
        savedPositions[savedCount++] = currentAngle;
        Serial.printf("Task F: Saved angle %d° at slot %d\n", currentAngle, savedCount);
        delay(500);
    }
    if (bit3 == LOW) {
        Serial.println("Task F: Starting robot activity...");
        for (int i=0; i<savedCount; i++) {
            writeServo(savedPositions[i], "Task F: Playing angle");
            delay(1000);
        }
    }
}

void taskG() {
    if (savedCount == 0) return;
    Serial.println("Task G: Playing stored sequence...");
    for (int i=0; i<savedCount; i++) {
        writeServo(savedPositions[i], "Task G: Servo");
        delay(2000);
    }
}

// --- Command helpers ---
void printSwitchStates() {
    Serial.printf("Switch states: Bit0=%s, Bit1=%s, Bit2=%s, Bit3=%s\n",
                  bit0 == LOW ? "LOW" : "HIGH",
                  bit1 == LOW ? "LOW" : "HIGH",
                  bit2 == LOW ? "LOW" : "HIGH",
                  bit3 == LOW ? "LOW" : "HIGH");
}

void printHelp() {
    Serial.println("Available commands:");
    Serial.println("  /help                   - Show this help message");
    Serial.println("  /task [1-7]             - Select task (a=1, b=2, ..., g=7)");
    Serial.println("  /task A-G");
    Serial.println("  /switch [2-3] [0/1]     - Set virtual switch (Bit2..Bit3) LOW=0, HIGH=1");
}

void handleCommand(String cmd) {
    int taskNum, pin, state;

    // Try to parse number version: /task 1-7
    if (sscanf(cmd.c_str(), "/task %d", &taskNum) == 1) {
        if (taskNum >= 1 && taskNum <= 7) {
            if (taskNum != currentTask) {
                currentTask = taskNum;
                Serial.printf("Selected task: %d\n", currentTask);
            }
        } else {
            Serial.println("Invalid task number! Use 1-7.");
        }

    // Try to parse letter version: /task A-G
    } else if (cmd.length() == 7 && cmd.substring(0,6).equalsIgnoreCase("/task ")) {
        char letter = toupper(cmd.charAt(6));
        if (letter >= 'A' && letter <= 'G') {
            taskNum = (letter - 'A') + 1; // Map A=1, B=2, ... G=7
            if (taskNum != currentTask) {
                currentTask = taskNum;
                Serial.printf("Selected task: %c (%d)\n", letter, currentTask);
            }
        } else {
            Serial.println("Invalid task letter! Use A-G.");
        }

    // Switch handling
    } else if (sscanf(cmd.c_str(), "/switch %d %d", &pin, &state) == 2) {
        int logicState = (state == 0 ? LOW : HIGH);
        bool changed = false;
        switch(pin) {
            case 2: if (bit2 != logicState) { bit2 = logicState; changed = true; } break;
            case 3: if (bit3 != logicState) { bit3 = logicState; changed = true; } break;
            default: Serial.println("Invalid switch number! Use 2-3."); return;
        }
        if (changed) {
            Serial.printf("Switch %d set to %s\n", pin, logicState==LOW?"LOW":"HIGH");
            printSwitchStates();
        }

    // Help
    } else if (cmd.equalsIgnoreCase("/help")) {
        printHelp();

    // Fallback
    } else {
        Serial.println("Invalid command! Type /help for list of commands.");
    }
}
