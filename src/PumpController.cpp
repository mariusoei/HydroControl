#include "PumpController.h"

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