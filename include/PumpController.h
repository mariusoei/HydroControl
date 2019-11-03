#ifndef PUMPCONTROLLER_H
#define PUMPCONTROLLER_H
 
#include <A4988.h>

#define MOTOR_STEPS 200
#define RPM 10
#define MICROSTEPS 1
#define DEG_PER_ML 90
#define MS1 10
#define MS2 11
#define MS3 12

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

void PumpController::updateController(float y){
    this->y = y;

    e = ref - y;
    // we can only lower the pH, so we limit the error at 0
    if(e>0) e=0;

    // Simple proportional control law
    u = K * e;

    // if using enable/disable on ENABLE pin (active LOW) instead of SLEEP uncomment next line
    stepper.setEnableActiveState(LOW);
    stepper.enable();

    // set the motor to move continuously for a reasonable time to hit the stopper
    // let's say 100 complete revolutions (arbitrary number)
    stepper.startRotate(u*DEG_PER_ML);                     // or in degrees
}

void PumpController::stepperUpdate(){
    unsigned wait_time_micros = stepper.nextAction();

    // 0 wait time indicates the motor has stopped
    if (wait_time_micros <= 0) {
        stepper.disable();       // comment out to keep motor powered
    }
}
 
#endif