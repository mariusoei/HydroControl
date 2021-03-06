#ifndef PUMPCONTROLLER_H
#define PUMPCONTROLLER_H
 
#include <A4988.h>

#define MOTOR_STEPS 200
#define RPM 60
#define MICROSTEPS 16


// #define MS1 D5
// #define MS2 D6
// #define MS3 D7

class PumpController
{
private:
    A4988 stepper;

    double K = -1; // Controller proportional gain (ml of ph-down per pH)
    double ref = 5.8; // Reference value
    double y; // Measurement
    double e; // Control error
    double u; // Control input (ml for next interval)
    double u_max = 3;
    double u_min = 0.2;

    const double deg_per_ml = 180/50*360.0; // Calibration value
public:
    PumpController(uint8_t pin_dir, uint8_t pin_step, uint8_t pin_sleep) : stepper(MOTOR_STEPS, pin_dir, pin_step, pin_sleep) {stepper.disable();}
    void updateController(double y);
    void stepperUpdate();
    double getLastControlInput(){return u;}
};
 
#endif