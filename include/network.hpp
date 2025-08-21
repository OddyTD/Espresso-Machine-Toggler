#pragma once

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <LittleFS.h>
#include <PubSubClient.h>

#include "credentials.hpp"

class NetworkConfig
{
public:
  void ConfigWiFi(const char *ssid = WIFI_SSID, const char *password = WIFI_PASSWORD);
};

class WebServerConfig
{
public:
  WebServerConfig();
  void begin();
  void handleClient();

private:
  ESP8266WebServer server;
};

class MQTTConfig
{
public:
  MQTTConfig();
  void begin();
  void loop();
  void publishState(bool state);

private:
  WiFiClient wifi;
  PubSubClient mqtt;
  static void onMessage(char *topic, uint8_t *payload, unsigned int length);
  void reconnect();
};
