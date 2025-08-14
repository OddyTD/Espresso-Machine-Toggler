#pragma once
#include <Arduino.h>
#include <EEPROM.h>

class eeprom
{
public:
  void begin();       // call once (EEPROM.begin)
  void load();        // read from EEPROM or initialize with defaults
  void save() const;  // write current PULSE_REVERSE + magic
  void clear() const; // clear magic so next boot reloads defaults
};
