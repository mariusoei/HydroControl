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

// Debug and Test options
#define _DEBUG_
//#define _TEST_

#ifdef _DEBUG_
#define _PP(a) Serial.print(a);
#define _PL(a) Serial.println(a);
#else
#define _PP(a)
#define _PL(a)
#endif


#define WIFI_CONNECTION_TIMEOUT 20
const char* ssid = "***REMOVED***";
const char* password = "***REMOVED***";

// Scheduler
Scheduler ts;

void task1Callback();
void task2Callback();

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

Task t1 (100 * TASK_MILLISECOND, TASK_FOREVER, &task1Callback, &ts, true);
Task t2 (TASK_IMMEDIATE, 100, &task2Callback, &ts, true);


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
#if defined(_DEBUG_) || defined(_TEST_)
  Serial.begin(115200);
  delay(2000);
  _PL("Scheduler Template: setup()");
#endif
}

void loop() {
  ts.execute();
}


void task1Callback() {
Serial.print(millis());
_PL(": task1Callback()");

}

void task2Callback() {
Serial.print(millis());
_PL(": task2Callb()");

}