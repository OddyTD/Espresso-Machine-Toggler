#include <Arduino.h>
#include "globals.hpp"
#include "calibration.hpp"
#include "network.hpp"

calibration cal;
NetworkConfig net;
WebServerConfig web;
MQTTConfig mqtt;

void servoControl()
{
  while (Serial.available())
  {
    char c = Serial.read();
    if (c == '\r' || c == '\n')
      continue;

    switch (c)
    {
    case '=':
      Serial.println(F("[SERVO/MESSAGE] (+) Jogging forward"));
      servo.forwardMove(JOG_MOVE_MS);
      break;
    case '-':
      Serial.println(F("[SERVO/MESSAGE] (-) Jogging backward"));
      servo.reverseMove(JOG_MOVE_MS);
      break;
    case 'e':
    case 'E':
      Serial.println(F("[SERVO/MESSAGE] (e) Clicking Espresso"));
      servo.clickEspresso();
      break;
    case 'c':
    case 'C':
      cal.begin();
      break;
    case 'p':
    case 'P':
      msg.printPulses();
      break;
    case 'r':
    case 'R':
      mem.clear();
      break;
    case 'h':
    case 'H':
      msg.showMenu();
      break;
    default:
      Serial.print(F("[ERREUR] Invalid option: '"));
      Serial.print(c);
      Serial.println(F("' Press 'h' for help."));
      break;
    }
  }
}

void setup()
{
  Serial.begin(115200);
  net.ConfigWiFi();
  mqtt.begin();
  web.begin();
  mem.begin();
  mem.load();
}

void loop()
{
  msg.showMenuOnce();
  mqtt.loop();
  web.handleClient();
  servoControl();
  yield();
}
