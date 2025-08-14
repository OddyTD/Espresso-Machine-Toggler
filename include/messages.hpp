#pragma once
#include <Arduino.h>

class messages
{
public:
  void showMenu();     // always prints
  void showMenuOnce(); // prints only the first time
  void printPulses();

private:
  bool menuShown = false;
};
