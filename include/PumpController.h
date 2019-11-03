#ifndef PUMPCONTROLLER_H
#define PUMPCONTROLLER_H
 
#include <A4988.h>

#define MOTOR_STEPS 200
#define DEG_PER_ML 90


class PumpController
{
private:
    A4988 *stepper;

    float K = -1; // Controller proportional gain (ml of ph-down per pH)
    float ref = 6.0f; // Reference value
    float y; // Measurement
    float u; // Control input (ml for next interval)

public:
    PumpController(A4988 *stepper) : stepper(stepper) {}
    void setMeasurement(float y) {this->y = y;}
    void updateController();
};

void PumpController::updateController(){
    float e = ref - y;
    // we can only lower the pH, so we limit the error at 0
    if(e>0) e=0;

    // Simple proportional control law
    u = K * e;

}
 
#endif