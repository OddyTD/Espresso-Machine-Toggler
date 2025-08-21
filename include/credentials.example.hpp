#pragma once

// Fill these in with your network info.
static const char* WIFI_SSID = "[Your_WiFi_SSID]";
static const char* WIFI_PASSWORD = "[Your_WiFi_Password]";

// Fill these in with your MQTT broker info.
static const char* MQTT_HOST = "[Your_MQTT_Broker_IP]";
static const int MQTT_PORT = 1883;
static const char* MQTT_USER = "[Your_MQTT_Username]";
static const char* MQTT_PASS = "[Your_MQTT_Password]";

// Fill these in with your topics info
static const char* TOPIC_CMD = "home/espresso/cmd";  // payload: "ON"/"OFF"
static const char* TOPIC_STAT = "home/espresso/stat";  // payload: "ON"/"OFF" (retain)