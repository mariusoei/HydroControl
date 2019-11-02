// #define _TASK_TIMECRITICAL      // Enable monitoring scheduling overruns
// #define _TASK_SLEEP_ON_IDLE_RUN // Enable 1 ms SLEEP_IDLE powerdowns between tasks if no callback methods were invoked during the pass
// #define _TASK_STATUS_REQUEST    // Compile with support for StatusRequest functionality - triggering tasks on status change events in addition to time only
// #define _TASK_WDT_IDS           // Compile with support for wdt control points and task ids
// #define _TASK_LTS_POINTER       // Compile with support for local task storage pointer
// #define _TASK_PRIORITY          // Support for layered scheduling priority
// #define _TASK_MICRO_RES         // Support for microsecond resolution
// #define _TASK_STD_FUNCTION      // Support for std::function (ESP8266 and ESP32 ONLY)
// #define _TASK_DEBUG             // Make all methods and variables public for debug purposes
// #define _TASK_INLINE            // Make all methods "inline" - needed to support some multi-tab, multi-file implementations
// #define _TASK_TIMEOUT           // Support for overall task timeout
// #define _TASK_OO_CALLBACKS      // Support for dynamic callback method binding
#include <TaskScheduler.h>

#include "temperatureLogger.h"
#include <ESP8266WiFi.h>


#define WIFI_CONNECTION_TIMEOUT 20
const char* ssid = "***REMOVED***";
const char* password = "***REMOVED***";

// Scheduler
Scheduler ts;

void publishWaterTemperature_callback();
void measureWaterTemperature_callback();
void updateOledDisplay_callback();

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
Task task_publishWaterTemperature (30 * TASK_SECOND, TASK_FOREVER, &publishWaterTemperature_callback, &ts, true);
Task task_measureWaterTemperature (5 * TASK_SECOND, TASK_FOREVER, &measureWaterTemperature_callback, &ts, true);
Task task_updateOledDisplay(200 * TASK_MILLISECOND, TASK_FOREVER, &updateOledDisplay_callback, &ts, true);


void setup_wifi() {
  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  for(int millis_waited=0; (millis_waited<1000*WIFI_CONNECTION_TIMEOUT)&&(WiFi.status() != WL_CONNECTED); millis_waited+=500){
    delay(500);
    Serial.print(".");
  }

  randomSeed(micros());

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}



void setup() {
    // put your setup code here, to run once:
    Serial.begin(115200);
    setup_wifi();
    setupTemperatureLogger();
    delay(100);
    Serial.println("Scheduler: setup complete");
}

void loop() {
    ts.execute();
}

void publishWaterTemperature_callback() {
    Serial.print(millis());
    Serial.println(": publishing water temperature");
    publishWaterTemperature();
}

void measureWaterTemperature_callback() {
    Serial.print(millis());
    Serial.println(": measuring water temperature");
    measureWaterTemperature();
}

void updateOledDisplay_callback() {
    Serial.print(millis());
    Serial.println(": updating OLED display");
    updateOledDisplay();
}