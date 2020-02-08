#include "SensorActuatorLogger.h"

#include <stdlib.h>

// Includes for the DS18B20 sensor
#include <OneWire.h>
#include <DallasTemperature.h>
// MQTT
#include <ESP8266WiFi.h>
#include <PubSubClient.h>


#include <AnalogPHMeter.h>
#include <EEPROM.h>

#include "RunningAverage.h"

// Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperatureC ICs)
OneWire oneWire(ONE_WIRE_BUS_TEMPSENSOR);

// Pass our oneWire reference to Dallas Temperature. 
DallasTemperature tempSensors(&oneWire);

const char* mqtt_server = "homeserver01.local";
const int mqtt_port = 1883;

const char* mqtt_user = "sensors";
const char* mqtt_password = "mariusmqttsensors12";
const String clientId = "HydroControl";

const char* mqtt_temperatureTopic = "hydrocontrol/sensor/temperatureC/sensor1";
const char* mqtt_temperatureTopicRaw = "hydrocontrol/sensor/temperatureC/sensor1/raw";
const char* mqtt_phTopic = "hydrocontrol/sensor/ph/sensor1";
const char* mqtt_phTopicRaw = "hydrocontrol/sensor/ph/sensor1/raw";
const char* mqtt_phTopicFiltered = "hydrocontrol/sensor/ph/sensor1/filtered";
const char* mqtt_controlInputTopic = "hydrocontrol/actuator/pump/u_ml";

const char* mqtt_ha_discovery_light_topic = "homeassistant/switch/hydrocontrol/led1/config";
const char* mqtt_ha_discovery_light_payload = R"({"name": "hydrocontrol_led1", "command_topic": "hydrocontrol/actuator/led1/set", "state_topic": "hydrocontrol/actuator/led1/state", "retain":"true"})";
const char* mqtt_lightSwitchCommandTopic = "hydrocontrol/actuator/led1/set";
const char* mqtt_lightSwitchStateTopic = "hydrocontrol/actuator/led1/state";

const char* mqtt_ha_discovery_fan_topic = "homeassistant/switch/hydrocontrol/fan1/config";
const char* mqtt_ha_discovery_fan_payload = R"({"name": "hydrocontrol_fan1", "command_topic": "hydrocontrol/actuator/fan1/set", "state_topic": "hydrocontrol/actuator/fan1/state", "retain":"true"})";
const char* mqtt_fanSwitchCommandTopic = "hydrocontrol/actuator/fan1/set";
const char* mqtt_fanSwitchSetLevelTopic = "hydrocontrol/actuator/fan1/level";
const char* mqtt_fanSwitchStateTopic = "hydrocontrol/actuator/fan1/state";

const char* mqtt_ha_discovery_phcontrol_topic = "homeassistant/switch/hydrocontrol/phcontrol/config";
const char* mqtt_ha_discovery_phcontrol_payload = R"({"name": "hydrocontrol_phcontrol", "command_topic": "hydrocontrol/actuator/phcontrol/set", "state_topic": "hydrocontrol/actuator/phcontrol/state", "retain":"true"})";
const char* mqtt_phControllerCommandTopic = "hydrocontrol/actuator/phcontrol/set";
const char* mqtt_phControllerStateTopic = "hydrocontrol/actuator/phcontrol/state";

const char* mqtt_switchPayloadOn = "ON";
const char* mqtt_switchPayloadOff = "OFF";

// Initial level of fan1 (0 - 1024)
int fan1_level = 200;

// PH Control enabled?
bool phControlActive = true;


WiFiClient espClient;
PubSubClient mqttClient(espClient);

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
float ph, ph_raw;
RunningAverage RA_ph(RUNNINGAVERAGE_N_POINTS_PH);
// Boolean for plausibility (only published if plausible)
bool phPlausible;
// Min and max plausible values
const float PH_MAX = 9.0f;
const float PH_MIN = 4.5f;
// Maximum allowed standard deviation within the buffer
const float PH_MAX_STDDEV = 0.1f;

bool MQTT_reconnectOnce() {
  // Loop until we're reconnected
  Serial.print("Attempting MQTT connection...");
  // Attempt to connect
  if (mqttClient.connect(clientId.c_str(),mqtt_user,mqtt_password)) {
    Serial.println("connected");
    mqttClient.loop();
    // Subscribe to command message topics
    mqttClient.subscribe(mqtt_lightSwitchCommandTopic);
    mqttClient.subscribe(mqtt_fanSwitchCommandTopic);
    mqttClient.subscribe(mqtt_fanSwitchSetLevelTopic);
    mqttClient.subscribe(mqtt_phControllerCommandTopic);
    // publish discovery messages
    Serial.println("Publishing HA discovery messages.");
    mqttClient.publish(mqtt_ha_discovery_light_topic, mqtt_ha_discovery_light_payload, true);
    mqttClient.publish(mqtt_ha_discovery_fan_topic, mqtt_ha_discovery_fan_payload, true);
    mqttClient.publish(mqtt_ha_discovery_phcontrol_topic, mqtt_ha_discovery_phcontrol_payload, true);
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

bool checkTempMeasurementPlausibility(double temperature_C){
  // Check the measured value for plausibility
  bool plausible = true;

  if(temperature_C>TEMP_MAX_C) plausible = false;
  if(temperature_C<TEMP_MIN_C) plausible = false;

  return plausible;
}

bool checkPHMeasurementPlausibility(double ph){
  // Check the measured value for plausibility
  bool plausible = true;

  if(ph>PH_MAX) plausible = false;
  if(ph<PH_MIN) plausible = false;
  if(RA_ph.getStandardDeviation() > PH_MAX_STDDEV) plausible = false;

  return plausible;
}

void mqttMessageReceived(char* topic, byte* payload, unsigned int length){
  // Callback for receiving MQTT messages

  if(!strcmp(topic,mqtt_lightSwitchCommandTopic)){
    // Handling LED1 switch commands
    if(!strncmp((char*)payload,mqtt_switchPayloadOn,length)){
      Serial.println("Activating LED lights.");
      digitalWrite(LED1_PIN,HIGH);
      mqttClient.publish(mqtt_lightSwitchStateTopic,mqtt_switchPayloadOn,true);
    }else if(!strncmp((char*)payload,mqtt_switchPayloadOff,length)){
      Serial.println("Deactivating LED lights.");
      digitalWrite(LED1_PIN,LOW);
      mqttClient.publish(mqtt_lightSwitchStateTopic,mqtt_switchPayloadOff,true);
    }
  } 
  if(!strcmp(topic,mqtt_fanSwitchCommandTopic)){
    // Handling FAN1 switch commands
    if(!strncmp((char*)payload,mqtt_switchPayloadOn,length)){
      Serial.println("Activating fan1.");
      analogWrite(FAN1_PIN,fan1_level);
      mqttClient.publish(mqtt_fanSwitchStateTopic,mqtt_switchPayloadOn);
    }else if(!strncmp((char*)payload,mqtt_switchPayloadOff,length)){
      Serial.println("Deactivating fan1.");
      digitalWrite(FAN1_PIN,LOW);
      mqttClient.publish(mqtt_fanSwitchStateTopic,mqtt_switchPayloadOff);
    }
  }  
  if(!strcmp(topic,mqtt_fanSwitchSetLevelTopic)){
    // Handling FAN1 level commands
    fan1_level = strtol((char*)payload,NULL,10);
    fan1_level = max(fan1_level,0);
    fan1_level = min(fan1_level,1024);
    Serial.print("Set fan1 level: ");
    Serial.println((char*)payload);
  }
  if(!strcmp(topic,mqtt_phControllerCommandTopic)){
    // Handling FAN1 switch commands
    if(!strncmp((char*)payload,mqtt_switchPayloadOn,length)){
      Serial.println("Activating ph control.");
      phControlActive = true;
      mqttClient.publish(mqtt_phControllerStateTopic,mqtt_switchPayloadOn);
    }else if(!strncmp((char*)payload,mqtt_switchPayloadOff,length)){
      Serial.println("Deactivating ph control.");
      phControlActive = false;
      mqttClient.publish(mqtt_phControllerStateTopic,mqtt_switchPayloadOff);
    }
  } 
}

void setupMQTT() {
  // Set calibration button pins as inputs
  pinMode(PHCAL_LOW_PIN,INPUT);
  pinMode(PHCAL_HIGH_PIN,INPUT);

  // Start up the sensor library
  tempSensors.begin();

  EEPROM.begin(sizeof(PHCalibrationValue));

  // Get pH sensor calibration from EEPROM
  struct PHCalibrationValue pHCalibrationValue;
  EEPROM.get(pHCalibrationValueAddress, pHCalibrationValue);
  pHSensor.initialize(pHCalibrationValue);

  // Setup mqtt
  mqttClient.setServer(mqtt_server, mqtt_port);
  mqttClient.setCallback(mqttMessageReceived);

  // Connect to the MQTT broker
  if (!mqttClient.connected()) { MQTT_reconnectOnce(); }

  // Set the output pin modes
  pinMode(LED1_PIN,OUTPUT);
  pinMode(FAN1_PIN,OUTPUT);
  phControlActive = true;
}

void loopMQTT(){
  if(!mqttClient.loop()){
    MQTT_reconnectOnce();
  }
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
  //Serial.print("Measuring ph value: ");
  // Read the sensor value
  ph_raw = pHSensor.singleReading().getpH();
  RA_ph.addValue(ph_raw);
  //Serial.print(ph_raw);

  ph = RA_ph.getAverage();
  //Serial.print(" (Average: "); Serial.print(ph); Serial.print(")");
  // Check plausibility
  phPlausible = checkPHMeasurementPlausibility(ph);
/*  if(phPlausible){
    Serial.println(" (plausible)");
  }else{
    Serial.println(" (not plausible!)");
  } */
  return ph;
}


void publishWaterTemperature() {
  publishDouble(mqtt_temperatureTopicRaw,temperatureC);
  if (temperaturePlausible){
    publishDouble(mqtt_temperatureTopic,temperatureC);
  }
}

void publishPH() {
  publishDouble(mqtt_phTopicRaw,ph_raw);
  publishDouble(mqtt_phTopicFiltered,ph);
  if (phPlausible){
    publishDouble(mqtt_phTopic,ph);
  }
}

void publishControlInput(double u){
  publishDouble(mqtt_controlInputTopic,u);
}

void publishDouble(const char* topic, double value) {
  if (!mqttClient.connected()) {
        // Only try once so other tasks aren't blocked
        MQTT_reconnectOnce();
  }
  if (mqttClient.connected()){
    mqttClient.loop();
    // Print value to character array
    char cstr [10];
    sprintf(cstr,"%.2f",value);
    Serial.print("Publishing to topic '");
    Serial.print(topic);
    Serial.print("', value: ");
    Serial.println(value);
    // publishDouble sensor value as MQTT message
    mqttClient.publish(topic, cstr);
  }
}

void phCalibrateLow() {
  Serial.println("Calibrating ph value (LOW)");
  pHSensor.calibrationLow(PHCAL_LOW_REF);
  EEPROM.put(pHCalibrationValueAddress, pHSensor.getCalibrationValue());
  EEPROM.commit();
}

void phCalibrateHigh() {
  Serial.println("Calibrating ph value (HIGH)");
  pHSensor.calibrationHigh(PHCAL_HIGH_REF);
  EEPROM.put(pHCalibrationValueAddress, pHSensor.getCalibrationValue());
  EEPROM.commit();
}
