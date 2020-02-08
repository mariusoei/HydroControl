#include "PumpController.h"

void PumpController::updateController(double y){
    stepper.begin(RPM, MICROSTEPS);
    this->y = y;

    // calculate the error - negative if ph is too high
    e = ref - y;
    // we can only lower the pH, so we limit the error at 0
    if(e>0) e=0;

    // Simple proportional control law (K is a negative constant for ph down)
    u = K * e;

    // //DEBUG
    // u = 1;

    // Upper and lower bounds for control input
    if(u>u_max) u=u_max; // not more than maximum
    if(u<u_min) u=0; // no input if requested below minimum

    // if using enable/disable on ENABLE pin (active LOW) instead of SLEEP uncomment next line
    stepper.setEnableActiveState(LOW);
    stepper.enable();

    // Start stepper motion
    Serial.print("Moving stepper (deg): ");
    Serial.println(u*deg_per_ml);
    stepper.rotate(u*deg_per_ml);
    stepper.disable();   
    // stepper.startRotate(u*DEG_PER_ML);
}

void PumpController::stepperUpdate(){
    unsigned wait_time_micros = stepper.nextAction();
    // Serial.print("Next motion in ");
    // Serial.println(wait_time_micros/1000.0f);

    // 0 wait time indicates the motor has stopped
    if (wait_time_micros <= 0) {
        stepper.disable();       // comment out to keep motor powered
    }
}