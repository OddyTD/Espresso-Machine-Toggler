#include <Arduino.h>
#include "globals.hpp"
#include "calibration.hpp"
#include "network.hpp"

calibration cal;
NetworkConfig net;
WebServerConfig web;

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
      Serial.println(F("[SERVO] Jog forward"));
      servo.forwardMove(JOG_MOVE_MS);
      break;
    case '-':
      Serial.println(F("[SERVO] Jog reverse"));
      servo.reverseMove(JOG_MOVE_MS);
      break;
    case 'e':
    case 'E':
      Serial.println(F("[SERVO] Espresso click"));
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
      Serial.print(F("Invalid option: '"));
      Serial.print(c);
      Serial.println(F("'. Press 'h' for help."));
      break;
    }
  }
}

void setup()
{
  Serial.begin(115200);
  net.ConfigWiFi();
  web.begin();
  mem.begin();
  mem.load();
}

void loop()
{
  msg.showMenuOnce();
  web.handleClient();
  servoControl();
  yield();
}
