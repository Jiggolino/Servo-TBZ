#ifndef PINGU_SERVO_H
#define PINGU_SERVO_H

#include <Arduino.h>
#include "driver/mcpwm.h"

class Pingu_Servo {
public:
    Pingu_Servo();  // Constructor

    void attach(int pin, int clamp_min_, int clamp_max_, bool invert_ = false);
    void write(int angle);
    void detach();
    int read();
    bool attached();
    void writeMicroseconds(int us);

private:
    int servoPin;
    int angle;
    int clamp_min, clamp_max;
    bool invert;
    mcpwm_unit_t mcpwm_unit;
    mcpwm_timer_t mcpwm_timer;
    mcpwm_operator_t mcpwm_operator;
    mcpwm_io_signals_t mcpwm_signal;
};

#endif
