#include "servo_action.hpp"
#include "globals.hpp"
#include <Arduino.h>

void ServoAction::attachWrite(int us)
{
  servo.attach(PIN_PWM);
  servo.writeMicroseconds(us);
}

void ServoAction::forwardMove(int ms)
{
  attachWrite(PULSE_FORWARD);
  delay(ms);
  servo.detach();
}

void ServoAction::reverseMove(int ms)
{
  attachWrite(PULSE_REVERSE);
  delay(ms);
  servo.detach();
}

void ServoAction::clickEspresso()
{
  forwardMove(MOVE_MS);
  delay(1000);
  reverseMove(MOVE_MS);
}
