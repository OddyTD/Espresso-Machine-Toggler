// include/globals.hpp
#pragma once
#include "eeprom.hpp"
#include "messages.hpp"
#include "servo_action.hpp"

// ===== Pin definitions =====
//#define D0 16
//#define D1 5
//#define D2 4
//#define D3 0
#define D4 2
//#define D5 14
//#define D6 12
//#define D7 13
//#define D8 15

// ===== Pins & timing =====
inline constexpr int PIN_PWM = D4;
inline constexpr int MOVE_MS = 250;
inline constexpr int JOG_MOVE_MS = 25;

// ----- Servo limits -----
inline constexpr int SERVO_MIN_US = 1000;
inline constexpr int SERVO_MAX_US = 2000;

// ----- Pulses -----
inline constexpr int PULSE_FORWARD = 1560; // fixed reference
inline constexpr int DEFAULT_REV_US = 1680;
inline int PULSE_REVERSE = DEFAULT_REV_US;
inline constexpr int NEUTRAL_REF_US = 1500;

// ===== Calibration params =====
inline constexpr int CAL_CYCLES = 5;
inline constexpr int CAL_ADJ_STEP_US = 2;
inline constexpr int CAL_COOLDOWN_MS = 500;

// ===== Shared objects (one per program) =====
inline eeprom mem;
inline messages msg;
inline ServoAction servo;
