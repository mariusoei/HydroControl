#include <TaskScheduler.h>

#include "SensorActuatorLogger.h"
#include <ESP8266WiFi.h>

#include "PumpController.h"

// For WifiManager library
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>         //https://github.com/tzapu/WiFiManager

#include <ArduinoOTA.h>

#define PIN_DIR D2
#define PIN_STEP D3
#define PIN_SLEEP D4

// PH Controller
PumpController phControl(PIN_DIR, PIN_STEP, PIN_SLEEP);

#define T_CONTROLLER (60*TASK_SECOND)

// Scheduler
Scheduler ts;




void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  WiFiManager wifiManager;
  wifiManager.autoConnect("AutoConnectAP_HydroControl");
  setupMQTT();
  delay(100);
  Serial.println("Scheduler: setup complete");
  // start the WiFi OTA library with internal (flash) based storage
  ArduinoOTA.setPassword("***REMOVED***");
  ArduinoOTA.begin();
}

void loop() {
  ts.execute();
}

void loopMQTT_callback(){
  Serial.print(millis());
  Serial.println(": running the MQTT update");
  loopMQTT();
}

void measureAndPublishWaterTemperature_callback() {
  Serial.print(millis());
  Serial.println(": measuring, publishing water temperature");
  measureWaterTemperature();
  publishWaterTemperature();
}

void publishPH_callback() {
  Serial.print(millis());
  Serial.println(": publishing ph value");
  publishPH();
}

void measurePH_callback(){
  measurePH();
}

void phControlUpdate_callback(){
  Serial.print(millis());
  Serial.println(": running pH controller update");
  float ph = measurePH();
  if(checkPHMeasurementPlausibility(ph)){
    phControl.updateController(ph);
    publishControlInput(phControl.getLastControlInput());
  }else{
    Serial.println("PH measurement implausible, not updating controller.");
  }
}

void phCalibrateButtonCheck_callback(){
  // Check both calibration buttons and perform calibration if one of them is pressed
  bool lowPressed = !((bool) digitalRead(PHCAL_LOW_PIN));
  bool highPressed = !((bool) digitalRead(PHCAL_HIGH_PIN));

  if(lowPressed && !highPressed) phCalibrateLow();
  if(highPressed && !lowPressed) phCalibrateHigh();
}

void otaHandle_callback(){
  ArduinoOTA.handle();
}


/*
  Scheduling defines:
  TASK_MILLISECOND
  TASK_SECOND
  TASK_MINUTE
  TASK_HOUR
  TASK_IMMEDIATE
  TASK_FOREVER
  TASK_ONCE
  TASK_NOTIMEOUT
*/
// Define all tasks
// Constructor: Task(Interval, Iterations/Repetitions, Callback, Scheduler, Enable)
Task task_measureAndPublishWaterTemperature (5 * TASK_SECOND, TASK_FOREVER, &measureAndPublishWaterTemperature_callback, &ts, true);
Task task_publishPH (5 * TASK_SECOND, TASK_FOREVER, &publishPH_callback, &ts, true);
Task task_measurePH (100 * TASK_MILLISECOND, TASK_FOREVER, &measurePH_callback, &ts, true);
Task task_phControlUpdate(T_CONTROLLER, TASK_FOREVER, &phControlUpdate_callback, &ts, true);
Task task_phCalibrateButtonCheck(200*TASK_MILLISECOND, TASK_FOREVER, &phCalibrateButtonCheck_callback, &ts, true);
Task task_otaHandle(10*TASK_MILLISECOND, TASK_FOREVER, &otaHandle_callback, &ts, true);
Task task_loopMQTT(1*TASK_SECOND, TASK_FOREVER, &loopMQTT_callback, &ts, true);