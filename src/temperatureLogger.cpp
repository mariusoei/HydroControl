#include "TemperatureLogger.h"

// Arduino header
#include <Arduino.h>
// Includes for the DS18B20 sensor
#include <OneWire.h>
#include <DallasTemperature.h>
// MQTT
#include <ESP8266WiFi.h>
#include <PubSubClient.h>


// DS18B20 data wire is plugged into port D2 on the ESP
#define ONE_WIRE_BUS_TEMPSENSOR D2
// Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperatureC ICs)
OneWire oneWire(ONE_WIRE_BUS_TEMPSENSOR);

// Pass our oneWire reference to Dallas Temperature. 
DallasTemperature sensors(&oneWire);

const char* mqtt_server = "homeserver01.local";
const int mqtt_port = 1883;

const char* mqtt_user = "sensors";
const char* mqtt_password = "mariusmqttsensors12";

const char* mqtt_sensorTopic = "home-assistant/sensor/temperatureC/sensor1";
const String clientId = "HydroControl";



const float TEMP_MAX_C = 50; // maximum realistic temperatureC
const float TEMP_MIN_C = -10; // and minimum (reject measurement if outside range)

WiFiClient espClient;
PubSubClient mqttClient(espClient);
long lastMsg = 0;
char msg[50];


// Wire library for I2C connection to OLED display
#include <Wire.h>  // Only needed for Arduino 1.6.5 and earlier
#include "SSD1306Wire.h" // legacy include: `#include "SSD1306.h"`

// Initialize the OLED display using Wire library
// D3 -> SDA
// D5 -> SCL
SSD1306Wire  display(0x3c, D3, D5);

// Variable for the sensor value (in deg C)
float temperatureC;
// And for the value printed to a character array
char temperature_cstr [10];
// Boolean for plausibility (only published if plausible)
bool temperaturePlausible;

// Measured calibration values for this specific sensor
const float TEMP_RAWHIGH = 99.2; // measured in boiling water
const float TEMP_RAWLOW = 1.0; // measured in ice bath
const float TEMP_REFHIGH = 99.2; // boiling water in Stuttgart
const float TEMP_REFLOW = 0; // ice bath reference
const float TEMP_RAW_RANGE = TEMP_RAWHIGH - TEMP_RAWLOW;
const float TEMP_REF_RANGE = TEMP_REFHIGH - TEMP_REFLOW;



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

float applySensorCalibration(float temperature_raw){
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


void setupTemperatureLogger() {
  // Start up the sensor library
  sensors.begin();

  // Initialising the UI will init the display too.
  display.init();

  display.flipScreenVertically();
  display.setFont(ArialMT_Plain_10);

  // Setup mqtt
  mqttClient.setServer(mqtt_server, mqtt_port);

}

void measureWaterTemperature(){
  // Read the temperatureC sensor value
  sensors.requestTemperatures(); // Send the command to get temperatures
  temperatureC = sensors.getTempCByIndex(0);
  temperatureC = applySensorCalibration(temperatureC);
  // Write temperatureC to character array
  sprintf(temperature_cstr,"%.2f",temperatureC);

  // Check plausibility
  temperaturePlausible = checkTempMeasurementPlausibility(temperatureC);
}

void updateOledDisplay(){
  // clear the display
  display.clear();
   
  display.setTextAlignment(TEXT_ALIGN_LEFT);
  display.setFont(ArialMT_Plain_24);
  display.drawString(0,0, String(temperatureC));
  // write the buffer to the display
  display.display();
}


void publishWaterTemperature() {
  if (!mqttClient.connected()) {
        // Only try once so the display update isn't blocked
        MQTT_reconnectOnce();
  }
  if (mqttClient.connected()&&temperaturePlausible){
    mqttClient.loop();
    // publish sensor value as MQTT message
    mqttClient.publish(mqtt_sensorTopic, temperature_cstr);
  }
}
