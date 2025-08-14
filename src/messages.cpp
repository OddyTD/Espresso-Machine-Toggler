#include "messages.hpp"
#include "globals.hpp"

void messages::showMenu()
{
  if (menuShown)
    return;
  Serial.println(F("========================================"));
  Serial.println(F("             Servo Control              "));
  Serial.println(F("========================================"));
  Serial.println(F("  = : Jog forward"));
  Serial.println(F("  - : Jog reverse"));
  Serial.println(F("  e : Espresso click"));
  Serial.println(F("  c : Calibrate"));
  Serial.println(F("  p : Print current pulse settings"));
  Serial.println(F("  r : Clear EEPROM (defaults on next boot)"));
  Serial.println(F("  h : Help"));
  Serial.println(F("========================================"));
  menuShown = true;
}

void messages::printPulses()
{
  Serial.print(F("[CFG] PULSE_FORWARD="));
  Serial.print(PULSE_FORWARD);
  Serial.print(F(" us, "));
  Serial.print(F("PULSE_REVERSE="));
  Serial.print(PULSE_REVERSE);
  Serial.println(F(" us"));
}

void messages::showMenuOnce()
{
  if (!menuShown)
  {
    showMenu();
  } // reuses your existing method
}
