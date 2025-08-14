#pragma once
#include <Servo.h>

class ServoAction
{
public:
  void attachWrite(int us);
  void forwardMove(int ms);
  void reverseMove(int ms);
  void clickEspresso();

private:
  Servo servo; // instance (not static)
};
