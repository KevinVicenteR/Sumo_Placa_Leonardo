#ifndef PINES_H
#define PINES_H

#include <Arduino.h>

// -- SENSORES DE PISO --
constexpr uint8_t S_PISO_IZQ = A1;
constexpr uint8_t S_PISO_DER = A2;
constexpr int BLANCO = 50;

// -- SENSORES DE ENEMIGO --
constexpr uint8_t S_FRONT_IZQ = 2;
constexpr uint8_t S_FRONT_CEN = 4;
constexpr uint8_t S_FRONT_DER = A5;
constexpr uint8_t S_LAT_IZQ   = 0; 
constexpr uint8_t S_LAT_DER   = A4;

// -- MOTORES --
constexpr uint8_t PWMA = 10;
constexpr uint8_t MA1A = 9;
constexpr uint8_t MA2A = 13;
constexpr uint8_t PWMB = 11;
constexpr uint8_t MA1B = 8;
constexpr uint8_t MA2B = 12;

// -- VELOCIDADES --
constexpr int16_t VelocidadMaxima = 180;
constexpr int16_t VelocidadMinima = 50;
constexpr int16_t VelocidadCurva = 105;
constexpr int16_t VelocidadGiro = 110;
constexpr int16_t VelocidadRetroceso = 180;
constexpr int16_t VelocidadAvance = 112;
constexpr int16_t VelocidadAtaqueFrontal = 135;
constexpr int16_t VelocidadBusquedaDer = 63;
constexpr int16_t VelocidadPivoteLateral = 110;
constexpr int16_t VelocidadCurvaAtaqueLateral = 70;

#endif
