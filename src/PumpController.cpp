#include "PumpController.h"

void PumpController::updateController(float y){
    stepper.begin(RPM, MICROSTEPS);
    this->y = y;

    e = ref - y;
    // we can only lower the pH, so we limit the error at 0
    if(e>0) e=0;

    // Simple proportional control law
    u = K * e;

    // if using enable/disable on ENABLE pin (active LOW) instead of SLEEP uncomment next line
    // stepper.setEnableActiveState(LOW);
    stepper.enable();

    // Start stepper motion
    Serial.print("Moving stepper (deg): ");
    Serial.println(u*DEG_PER_ML);
    stepper.rotate(u*DEG_PER_ML);
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