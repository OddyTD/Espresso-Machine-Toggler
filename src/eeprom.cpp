#include <EEPROM.h>
#include "eeprom.hpp"
#include "globals.hpp"

static const uint32_t MAGIC = 0xDEADBEEF;
static const int ADDR_MAGIC = 0;
static const int ADDR_REVERSE = 4;
static const int EEPROM_SIZE = 8;

void eeprom::begin() { EEPROM.begin(EEPROM_SIZE); }

void eeprom::save() const
{
  EEPROM.put(ADDR_MAGIC, MAGIC);
  EEPROM.put(ADDR_REVERSE, (int32_t)PULSE_REVERSE);
  EEPROM.commit();
  Serial.println(F("[EEPROM] Saved calibration."));
}

void eeprom::clear() const
{
  uint32_t zero = 0;
  EEPROM.put(ADDR_MAGIC, zero);
  EEPROM.commit();
  Serial.println(F("[EEPROM] Cleared. Defaults will load on next boot."));
}

void eeprom::load()
{
  uint32_t magic = 0;
  EEPROM.get(ADDR_MAGIC, magic);
  if (magic == MAGIC)
  {
    int32_t rev;
    EEPROM.get(ADDR_REVERSE, rev);
    if (rev < SERVO_MIN_US || rev > SERVO_MAX_US)
      rev = DEFAULT_REV_US;
    PULSE_REVERSE = rev;
    Serial.println(F("[EEPROM] Loaded calibration."));
  }
  else
  {
    PULSE_REVERSE = DEFAULT_REV_US;
    save();
    Serial.println(F("[EEPROM] Initialized with defaults."));
  }
}
