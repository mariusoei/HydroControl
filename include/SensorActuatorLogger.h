#ifndef SENSORACTUATORLOGGER_H
#define SENSORACTUATORLOGGER_H

// DS18B20 data wire is plugged into port D1 on the ESP
#define ONE_WIRE_BUS_TEMPSENSOR D1

// Analog input pin for pH measurement
#define PHSENSOR_PIN A0
#define PHCAL_LOW_PIN D6
#define PHCAL_HIGH_PIN D5



void setupMQTT();

void publishWaterTemperature();
float measureWaterTemperature();
void publishPH();
float measurePH();
void publishControlInput(double u);

bool checkPHMeasurementPlausibility(double ph);
bool checkTempMeasurementPlausibility(double temperature_C);

void publish(const char* topic, double value);

void phCalibrateLow();
void phCalibrateHigh();

#endif