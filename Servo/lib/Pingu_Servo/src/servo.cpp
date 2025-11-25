#include "Pingu_Servo.h"

// Constructor
Pingu_Servo::Pingu_Servo() 
    : servoPin(-1), angle(0), clamp_min(0), clamp_max(180), invert(false),
      mcpwm_unit(MCPWM_UNIT_0), mcpwm_timer(MCPWM_TIMER_0), 
      mcpwm_operator(MCPWM_OPR_A), mcpwm_signal(MCPWM0A) {}

// Attach Servo
void Pingu_Servo::attach(int pin, int clamp_min_, int clamp_max_, bool invert_) {
    this->invert = invert_;
    this->servoPin = pin;
    this->clamp_min = clamp_min_;
    this->clamp_max = clamp_max_;

    // Configure MCPWM GPIO
    mcpwm_gpio_init(mcpwm_unit, mcpwm_signal, this->servoPin);

    // Configure MCPWM
    mcpwm_config_t pwm_config = {};
    pwm_config.frequency = 50; // 50Hz PWM for Servo
    pwm_config.cmpr_a = 7.5;   // 7.5% duty cycle (neutral)
    pwm_config.cmpr_b = 0;
    pwm_config.counter_mode = MCPWM_UP_COUNTER;
    pwm_config.duty_mode = MCPWM_DUTY_MODE_0; // Normal mode

    mcpwm_init(mcpwm_unit, mcpwm_timer, &pwm_config);
}

// Write angle to servo
void Pingu_Servo::write(int angle) {
    if (servoPin != -1) {
        this->angle = constrain(angle, clamp_min, clamp_max);

        // Convert angle to duty cycle (2.5% - 12.5% range for servo control)
        float dutyCycle = map(this->angle, 0, 180, 2.5 * 100, 12.5 * 100) / 100.0;

        if (invert) {
            // Proper signal inversion: Swap HIGH and LOW times
            mcpwm_set_duty_type(mcpwm_unit, mcpwm_timer, mcpwm_operator, MCPWM_DUTY_MODE_1);
        } else {
            mcpwm_set_duty_type(mcpwm_unit, mcpwm_timer, mcpwm_operator, MCPWM_DUTY_MODE_0);
        }

        // Apply the duty cycle
        mcpwm_set_duty(mcpwm_unit, mcpwm_timer, mcpwm_operator, dutyCycle);
    }
}

// Detach the servo
void Pingu_Servo::detach() {
    mcpwm_stop(mcpwm_unit, mcpwm_timer);
    this->servoPin = -1;
}

// Read the last written angle
int Pingu_Servo::read() {
    return this->angle;
}

// Check if servo is attached
bool Pingu_Servo::attached() {
    return (this->servoPin != -1);
}

// Write microseconds (alternative control)
void Pingu_Servo::writeMicroseconds(int us) {
    float dutyCycle = map(us, 500, 2500, 2.5 * 100, 12.5 * 100) / 100.0;

    if (invert) {
        mcpwm_set_duty_type(mcpwm_unit, mcpwm_timer, mcpwm_operator, MCPWM_DUTY_MODE_1);
    } else {
        mcpwm_set_duty_type(mcpwm_unit, mcpwm_timer, mcpwm_operator, MCPWM_DUTY_MODE_0);
    }

    mcpwm_set_duty(mcpwm_unit, mcpwm_timer, mcpwm_operator, dutyCycle);
}
