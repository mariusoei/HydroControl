#ifndef SENSORLOGGER_H
#define SENSORLOGGER_H

// DS18B20 data wire is plugged into port D1 on the ESP
#define ONE_WIRE_BUS_TEMPSENSOR D1

// Analog input pin for pH measurement
#define PHSENSOR_PIN A0

// OLED pin mapping
#define OLED_PIN_SDA D3
#define OLED_PIN_SCL D5


void setupLogger();

void publishWaterTemperature();
void measureWaterTemperature();
void publishPH();
void measurePH();
void updateOledDisplay();


#endif