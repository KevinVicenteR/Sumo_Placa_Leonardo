#ifndef PINES_H
#define PINES_H

#include <Arduino.h>

// Sensores de piso (analógicos)
constexpr uint8_t S_PISO_IZQ  = A1;
constexpr uint8_t S_PISO_DER  = A2;

// Sensores de enemigo (digitales)
// Nota: el pin 1 es TX de Serial1; no usar Serial1 mientras esté conectado aquí.
constexpr uint8_t S_FRONT_IZQ = 2;
constexpr uint8_t S_FRONT_CEN = 4;
constexpr uint8_t S_FRONT_DER = A5;
constexpr uint8_t S_LAT_IZQ   = 1;
constexpr uint8_t S_LAT_DER   = A4;

// Driver de motores
// Nota: el pin 13 es el LED integrado; el bootloader lo hace parpadear tras el reset.
constexpr uint8_t PWMA = 10;
constexpr uint8_t MA1A = 9;
constexpr uint8_t MA2A = 13;
constexpr uint8_t PWMB = 11;
constexpr uint8_t MA1B = 8;
constexpr uint8_t MA2B = 12;

#endif
