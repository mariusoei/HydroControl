#ifndef SENSORLOGGER_H
#define SENSORLOGGER_H

void setupLogger();

void publishWaterTemperature();
void measureWaterTemperature();
void publishPH();
void measurePH();
void updateOledDisplay();


#endif