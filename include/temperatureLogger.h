#ifndef TEMPERATURELOGGER_H
#define TEMPERATURELOGGER_H

void setupTemperatureLogger();
void loopTemperatureLogger();

bool checkMeasurementPlausibility(float temperature_C);
float sensorCalibration(float temperature_raw);
void reconnect();
bool reconnectOnce();
void setup_wifi();


#endif