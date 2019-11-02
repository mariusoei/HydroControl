#include "temperatureLogger.h"

// Arduino header
#include <Arduino.h>
// Includes for the DS18B20 sensor
#include <OneWire.h>
#include <DallasTemperature.h>
// MQTT
#include <PubSubClient.h>


// DS18B20 data wire is plugged into port D2 on the ESP
#define ONE_WIRE_BUS_TEMPSENSOR D2
// Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
OneWire oneWire(ONE_WIRE_BUS_TEMPSENSOR);

// Pass our oneWire reference to Dallas Temperature. 
DallasTemperature sensors(&oneWire);

const char* mqtt_server = "homeserver01.local";
const int mqtt_port = 1883;

const char* mqtt_user = "sensors";
const char* mqtt_password = "mariusmqttsensors12";

const char* mqtt_sensorTopic = "home-assistant/sensor/temperature/sensor1";
const String clientId = "TempSensor1";

const int interval_publish = 15; //publish every 15s
uint32_t last_millis_publish=0;



const float temp_max_C = 50; // maximum realistic temperature
const float temp_min_C = -10; // and minimum (reject measurement if outside range)

WiFiClient espClient;
PubSubClient client(espClient);
long lastMsg = 0;
char msg[50];


// Wire library for I2C connection to OLED display
#include <Wire.h>  // Only needed for Arduino 1.6.5 and earlier
#include "SSD1306Wire.h" // legacy include: `#include "SSD1306.h"`

// Initialize the OLED display using Wire library
// D3 -> SDA
// D5 -> SCL
SSD1306Wire  display(0x3c, D3, D5);

// Variable for the sensor value
float temperature;




bool reconnectOnce() {
  // Loop until we're reconnected
  Serial.print("Attempting MQTT connection...");
  // Attempt to connect
  if (client.connect(clientId.c_str(),mqtt_user,mqtt_password)) {
    Serial.println("connected");
    return true;
  } else {
    Serial.print("failed, rc=");
    Serial.print(client.state());
    return false;
  }
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    if(reconnectOnce()) return;
    Serial.println(" try again in 5 seconds");
    // Wait 5 seconds before retrying
    delay(5000);
  }
}

float sensorCalibration(float temperature_raw){
  // Use the measured values from ice bath and boiling water for sensor calibration
  // Obviously only valid for one specific sensor
  float RawHigh = 99.2; // measured in boiling water
  float RawLow = 1.0; // measured in ice bath
  float ReferenceHigh = 99.2; // boiling water in Stuttgart
  float ReferenceLow = 0; // ice bath reference
  float RawRange = RawHigh - RawLow;
  float ReferenceRange = ReferenceHigh - ReferenceLow;
  float CorrectedValue = (((temperature_raw - RawLow) * ReferenceRange) / RawRange) + ReferenceLow;

  return CorrectedValue;
}

bool checkMeasurementPlausibility(float temperature_C){
  // Check the measured value for plausibility
  bool plausible = true;

  if(temperature_C>temp_max_C) plausible = false;
  if(temperature_C<temp_min_C) plausible = false;

  return plausible;
}


void setupTemperatureLogger() {
  Serial.begin(9600);
  Serial.println();
  Serial.println();

  // Start up the sensor library
  sensors.begin();

  // Initialising the UI will init the display too.
  display.init();

  display.flipScreenVertically();
  display.setFont(ArialMT_Plain_10);

  // Setup wifi and mqtt
  setup_wifi();
  client.setServer(mqtt_server, mqtt_port);

}


void loopTemperatureLogger() {

  // Read the temperature sensor value
  sensors.requestTemperatures(); // Send the command to get temperatures
  temperature = sensors.getTempCByIndex(0);
  temperature = sensorCalibration(temperature);
  // Write temperature to character array
  char temperature_cstr [10];
  sprintf(temperature_cstr,"%.2f",temperature);

  // Check plausibility
  bool plausible = checkMeasurementPlausibility(temperature);

  // clear the display
  display.clear();
   
  display.setTextAlignment(TEXT_ALIGN_LEFT);
  display.setFont(ArialMT_Plain_24);
  display.drawString(0,0, String(temperature));
  // write the buffer to the display
  display.display();

  if (!client.connected()) {
        // Only try once so the display update isn't blocked
        reconnectOnce();
  }
  if (client.connected()){
    client.loop();
    // don't update mqtt publication every second
    if(millis()>=last_millis_publish+1000*interval_publish){
      last_millis_publish = millis();
      // publish sensor value as MQTT message
      if (plausible) client.publish(mqtt_sensorTopic, temperature_cstr);
    }
  }
  delay(200);
}
