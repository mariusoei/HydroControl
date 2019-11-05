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

#include "SensorLogger.h"
#include <ESP8266WiFi.h>

#include "PumpController.h"


#define WIFI_CONNECTION_TIMEOUT 20
const char* ssid = "***REMOVED***";
const char* password = "***REMOVED***";

#define PIN_DIR 1
#define PIN_STEP 2
#define PIN_SLEEP 3

// PH Controller
PumpController phControl(PIN_DIR, PIN_STEP, PIN_SLEEP);

// Scheduler
Scheduler ts;

void measureAndPublishWaterTemperature_callback();
void measureAndPublishPH_callback();
void updateOledDisplay_callback();

void phControlUpdate_callback();
void phControlStepperAction_callback();

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
Task task_measureAndPublishWaterTemperature (30 * TASK_SECOND, TASK_FOREVER, &measureAndPublishWaterTemperature_callback, &ts, true);
Task task_measureAndPublishPH (60 * TASK_SECOND, TASK_FOREVER, &measureAndPublishPH_callback, &ts, true);
Task task_updateOledDisplay(200 * TASK_MILLISECOND, TASK_FOREVER, &updateOledDisplay_callback, &ts, true);
Task task_phControlUpdate(5 * TASK_MINUTE, TASK_FOREVER, &phControlUpdate_callback, &ts, true);
Task task_phControlStepperAction(TASK_MILLISECOND, TASK_FOREVER, &phControlStepperAction_callback, &ts, true);


void setup_wifi() {
  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while(WiFi.status() != WL_CONNECTED){
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
    setupLogger();
    delay(100);
    Serial.println("Scheduler: setup complete");
}

void loop() {
    ts.execute();
}

void measureAndPublishWaterTemperature_callback() {
    Serial.print(millis());
    Serial.println(": measuring, publishing water temperature");
    measureWaterTemperature();
    publishWaterTemperature();
}

void measureAndPublishPH_callback() {
    Serial.print(millis());
    Serial.println(": measuring, publishing ph value");
    measurePH();
    publishPH();
}

void updateOledDisplay_callback() {
    Serial.print(millis());
    Serial.println(": updating OLED display");
    updateOledDisplay();
}

void phControlUpdate_callback(){
    Serial.print(millis());
    Serial.println(": running pH controller update");
    //TODO: use measurement value for pH
    float ph = 6.5;
    phControl.updateController(ph);
}

void phControlStepperAction_callback(){
    Serial.print(millis());
    Serial.println(": stepper action");
    phControl.stepperUpdate();
}