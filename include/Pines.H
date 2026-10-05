#ifndef PINES_H
#define PINES_H

#include <Arduino.h>

constexpr uint8_t S_PISO_IZQ  = A1;
constexpr uint8_t S_PISO_DER  = A2;

constexpr uint8_t S_FRONT_IZQ = 2;
constexpr uint8_t S_FRONT_CEN = 4;
constexpr uint8_t S_FRONT_DER = A5;
constexpr uint8_t S_LAT_IZQ   = 1;
constexpr uint8_t S_LAT_DER   = A4;

constexpr uint8_t PWMA = 10;
constexpr uint8_t MA1A = 9;
constexpr uint8_t MA2A = 13;
constexpr uint8_t PWMB = 11;
constexpr uint8_t MA1B = 8;
constexpr uint8_t MA2B = 12;

constexpr int PIN_MODULO_ARRANQUE = -1;

constexpr uint8_t DIP_1 = 5;
constexpr uint8_t DIP_2 = 6;
constexpr uint8_t DIP_3 = 7;

#endif
