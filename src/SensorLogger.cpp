#include "SensorLogger.h"

// Includes for the DS18B20 sensor
#include <OneWire.h>
#include <DallasTemperature.h>
// MQTT
#include <ESP8266WiFi.h>
#include <PubSubClient.h>


#include <AnalogPHMeter.h>
#include <EEPROM.h>

// Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperatureC ICs)
OneWire oneWire(ONE_WIRE_BUS_TEMPSENSOR);

// Pass our oneWire reference to Dallas Temperature. 
DallasTemperature tempSensors(&oneWire);

const char* mqtt_server = "homeserver01.local";
const int mqtt_port = 1883;

const char* mqtt_user = "sensors";
const char* mqtt_password = "mariusmqttsensors12";

const char* mqtt_temperatureTopic = "hydrocontrol/sensor/temperatureC/sensor1";
const char* mqtt_phTopic = "hydrocontrol/sensor/ph/sensor1";
const String clientId = "HydroControl";

WiFiClient espClient;
PubSubClient mqttClient(espClient);
long lastMsg = 0;
char msg[50];

// Variable for the sensor value (in deg C)
float temperatureC;
// And for the value printed to a character array
char temperature_cstr [10];
// Boolean for plausibility (only published if plausible)
bool temperaturePlausible;

// Measured calibration values for this specific temperature sensor
const float TEMP_RAWHIGH = 99.2; // measured in boiling water
const float TEMP_RAWLOW = 1.0; // measured in ice bath
const float TEMP_REFHIGH = 99.2; // boiling water in Stuttgart
const float TEMP_REFLOW = 0; // ice bath reference
const float TEMP_RAW_RANGE = TEMP_RAWHIGH - TEMP_RAWLOW;
const float TEMP_REF_RANGE = TEMP_REFHIGH - TEMP_REFLOW;

const float TEMP_MAX_C = 50; // maximum realistic temperatureC
const float TEMP_MIN_C = -10; // and minimum (reject measurement if outside range)


AnalogPHMeter pHSensor(PHSENSOR_PIN);
const unsigned int pHCalibrationValueAddress = 0;

#define PHCAL_HIGH_REF 6.88f
#define PHCAL_LOW_REF 4.00f

// Variable for the ph sensor value
float ph;
// And for the value printed to a character array
char ph_cstr [10];
// Boolean for plausibility (only published if plausible)
bool phPlausible;
// Min and max plausible values
const float PH_MAX = 9.0f;
const float PH_MIN = 4.5f;

bool MQTT_reconnectOnce() {
  // Loop until we're reconnected
  Serial.print("Attempting MQTT connection...");
  // Attempt to connect
  if (mqttClient.connect(clientId.c_str(),mqtt_user,mqtt_password)) {
    Serial.println("connected");
    return true;
  } else {
    Serial.print("failed, rc=");
    Serial.print(mqttClient.state());
    return false;
  }
}

void MQTT_reconnect() {
  // Loop until we're reconnected
  while (!mqttClient.connected()) {
    if(MQTT_reconnectOnce()) return;
    Serial.println(" try again in 5 seconds");
    // Wait 5 seconds before retrying
    delay(5000);
  }
}

float applyTemperatureSensorCalibration(float temperature_raw){
  // Use the measured values from ice bath and boiling water for sensor calibration
  float CorrectedValue = (((temperature_raw - TEMP_RAWLOW) * TEMP_REF_RANGE) / TEMP_RAW_RANGE) + TEMP_REFLOW;
  return CorrectedValue;
}

bool checkTempMeasurementPlausibility(float temperature_C){
  // Check the measured value for plausibility
  bool plausible = true;

  if(temperature_C>TEMP_MAX_C) plausible = false;
  if(temperature_C<TEMP_MIN_C) plausible = false;

  return plausible;
}

bool checkPHMeasurementPlausibility(float ph){
  // Check the measured value for plausibility
  bool plausible = true;

  if(ph>PH_MAX) plausible = false;
  if(ph<PH_MIN) plausible = false;

  return plausible;
}

void setupLogger() {
  // Set calibration button pins as inputs
  pinMode(PHCAL_LOW_PIN,INPUT);
  pinMode(PHCAL_HIGH_PIN,INPUT);

  // Start up the sensor library
  tempSensors.begin();

  // Get pH sensor calibration from EEPROM
  struct PHCalibrationValue pHCalibrationValue;
  EEPROM.get(pHCalibrationValueAddress, pHCalibrationValue);
  pHSensor.initialize(pHCalibrationValue);

  // Setup mqtt
  mqttClient.setServer(mqtt_server, mqtt_port);

}

float measureWaterTemperature(){
  // Read the temperatureC sensor value
  Serial.print("Measuring water temperature: ");
  tempSensors.requestTemperatures(); // Send the command to get temperatures
  temperatureC = tempSensors.getTempCByIndex(0);
  temperatureC = applyTemperatureSensorCalibration(temperatureC);
  Serial.print(temperatureC);
  // Write temperatureC to character array
  sprintf(temperature_cstr,"%.2f",temperatureC);

  // Check plausibility
  temperaturePlausible = checkTempMeasurementPlausibility(temperatureC);
  if(temperaturePlausible){
    Serial.println(" (plausible)");
  }else{
    Serial.println(" (not plausible!)");
  }
  return temperatureC;
}

float measurePH(){
  Serial.print("Measuring ph value: ");
  // Read the sensor value
  ph = pHSensor.singleReading().getpH();
  Serial.print(ph);
  // Write ph value to character array
  sprintf(ph_cstr,"%.2f",ph);
  // Check plausibility
  phPlausible = checkPHMeasurementPlausibility(ph);
  if(phPlausible){
    Serial.println(" (plausible)");
  }else{
    Serial.println(" (not plausible!)");
  }
  return ph;
}


void publishWaterTemperature() {
  if (!mqttClient.connected()) {
        // Only try once so the display update isn't blocked
        MQTT_reconnectOnce();
  }
  if (mqttClient.connected()&&temperaturePlausible){
    mqttClient.loop();
    // publish sensor value as MQTT message
    mqttClient.publish(mqtt_temperatureTopic, temperature_cstr);
  }
}

void publishPH() {
  if (!mqttClient.connected()) {
        // Only try once so the display update isn't blocked
        MQTT_reconnectOnce();
  }
  if (mqttClient.connected()&&phPlausible){
    mqttClient.loop();
    // publish sensor value as MQTT message
    mqttClient.publish(mqtt_phTopic, ph_cstr);
  }
}


void phCalibrateLow() {
  Serial.println("Calibrating ph value (LOW)");
  pHSensor.calibrationLow(PHCAL_LOW_REF);
  EEPROM.put(pHCalibrationValueAddress, pHSensor.getCalibrationValue());
}

void phCalibrateHigh() {
  Serial.println("Calibrating ph value (HIGH)");
  pHSensor.calibrationMid(PHCAL_HIGH_REF);
  EEPROM.put(pHCalibrationValueAddress, pHSensor.getCalibrationValue());
}
