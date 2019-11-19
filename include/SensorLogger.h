#ifndef SENSORLOGGER_H
#define SENSORLOGGER_H

// DS18B20 data wire is plugged into port D1 on the ESP
#define ONE_WIRE_BUS_TEMPSENSOR D1

// Analog input pin for pH measurement
#define PHSENSOR_PIN A0
#define PHCAL_LOW_PIN D6
#define PHCAL_HIGH_PIN D5



void setupLogger();

void publishWaterTemperature();
float measureWaterTemperature();
void publishPH();
float measurePH();

void phCalibrateLow();
void phCalibrateHigh();

#endif