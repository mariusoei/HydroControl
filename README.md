# HydroControl
HydroControl is an ESP8266/ESP32-based controller for hydroponic plant cultivation.

It is currently in a very initial stage and things might go horribly wrong.
The code is licensed under the Creative Commons Zero v1.0 Universal license.

## Features
Not many, at this stage:
- MQTT relay control for lighting
- pH control based on a cheap pH meter and a peristaltic pump (consisting of a stepper motor and mostly 3d printed parts)
- Reservoir temperature measurement with DS18B20
- Smoothing, plausiblity check and MQTT reporting of measured values
- Homeassistant auto-discovery for MQTT components
- OTA flashing/updating with PlatformIO/ArduinoOTA
