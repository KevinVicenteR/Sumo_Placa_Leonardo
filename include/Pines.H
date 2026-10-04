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

// Indicadores de usuario integrados, según la biblioteca XMotion.
constexpr uint8_t LED_IZQ = 8;
constexpr uint8_t LED_DER = 9;

// XMotion: biblioteca oficial de JSUMO (LM_PWM=3, LM_DIR=12,
// RM_PWM=11, RM_DIR=13). Una dirección por motor, HIGH = avance.
constexpr uint8_t PWMA = 3;
constexpr uint8_t DIR_IZQ = 12;
constexpr uint8_t PWMB = 11;
constexpr uint8_t DIR_DER = 13;

#endif
