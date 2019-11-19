#ifndef SENSORLOGGER_H
#define SENSORLOGGER_H

// DS18B20 data wire is plugged into port D1 on the ESP
#define ONE_WIRE_BUS_TEMPSENSOR D1

// Analog input pin for pH measurement
#define PHSENSOR_PIN A0
#define PHCAL_LOW_PIN D6
#define PHCAL_HIGH_PIN D5

//TODO: Insert correct values
#define PHCAL_HIGH_REF 6.8f
#define PHCAL_LOW_REF 4.0f

// OLED pin mapping
// #define OLED_PIN_SDA D3
// #define OLED_PIN_SCL D5


void setupLogger();

void publishWaterTemperature();
void measureWaterTemperature();
void publishPH();
void measurePH();
// void updateOledDisplay();

void phCalibrateLow();
void phCalibrateHigh();

#endif