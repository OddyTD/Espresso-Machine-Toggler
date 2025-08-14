#include <Arduino.h>
#include "globals.hpp"
#include "calibration.hpp"

void calibration::begin()
{
  Serial.println(F("[CAL] Starting calibration cycles..."));
  msg.printPulses();

  for (int i = 0; i < CAL_CYCLES; ++i)
  {
    Serial.print('[');
    Serial.print(i + 1);
    Serial.print('/');
    Serial.print(CAL_CYCLES);
    Serial.println(F("] pass..."));
    servo.clickEspresso();
    delay(CAL_COOLDOWN_MS);
  }

  Serial.println(F("[CAL] Drift result? Reply with:"));
  Serial.println(F("  B = ended backward → REVERSE closer to 1500"));
  Serial.println(F("  F = ended forward  → REVERSE farther from 1500"));
  Serial.println(F("  N = no noticeable drift"));
  Serial.print(F("  > "));

  while (!Serial.available())
  {
  }
  char ans = Serial.read();
  while (Serial.available())
    Serial.read();

  if (ans == 'b' || ans == 'B')
  {
    if (PULSE_REVERSE > NEUTRAL_REF_US)
      PULSE_REVERSE -= CAL_ADJ_STEP_US;
    else
      PULSE_REVERSE += CAL_ADJ_STEP_US;
    Serial.println(F("[CAL] Adjusted REVERSE closer to 1500."));
  }
  else if (ans == 'f' || ans == 'F')
  {
    if (PULSE_REVERSE > NEUTRAL_REF_US)
      PULSE_REVERSE += CAL_ADJ_STEP_US;
    else
      PULSE_REVERSE -= CAL_ADJ_STEP_US;
    Serial.println(F("[CAL] Adjusted REVERSE farther from 1500."));
  }
  else
  {
    Serial.println(F("[CAL] No change."));
  }

  PULSE_REVERSE = constrain(PULSE_REVERSE, SERVO_MIN_US, SERVO_MAX_US);
  msg.printPulses();
  mem.save();
  Serial.println(F("[CAL] Saved. Run 'c' again to refine if needed."));
}
