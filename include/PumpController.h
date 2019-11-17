#ifndef PUMPCONTROLLER_H
#define PUMPCONTROLLER_H
 
#include <A4988.h>

#define MOTOR_STEPS 200
#define RPM 10
#define MICROSTEPS 1
#define DEG_PER_ML 90


#define MS1 D5
#define MS2 D6
#define MS3 D7

class PumpController
{
private:
    A4988 stepper;

    float K = -1; // Controller proportional gain (ml of ph-down per pH)
    float ref = 6.0f; // Reference value
    float y; // Measurement
    float e; // Control error
    float u; // Control input (ml for next interval)

public:
    PumpController(uint8_t pin_dir, uint8_t pin_step, uint8_t pin_sleep) : stepper(MOTOR_STEPS, pin_dir, pin_step, pin_sleep, MS1, MS2, MS3) {stepper.begin(RPM, MICROSTEPS);}
    void updateController(float y);
    void stepperUpdate();
};
 
#endif