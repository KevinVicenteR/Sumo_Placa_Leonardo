// =============================================================================
// Pines.h — Mapa de conexiones de la placa JSumo XMotion (Arduino Leonardo)
// -----------------------------------------------------------------------------
// Único lugar donde se dice qué está conectado a cada pin. Si se cambia un
// cable de sitio, solo hay que tocar este archivo.
// =============================================================================
#ifndef PINES_H
#define PINES_H

#include <Arduino.h>

// --- Sensores de piso (analógicos): ven la línea blanca del borde del dohyo ---
constexpr uint8_t S_PISO_IZQ  = A1;
constexpr uint8_t S_PISO_DER  = A2;

// --- Sensores de enemigo (digitales): detectan al robot rival ---
// FRONT_IZQ y FRONT_DER miran a 45°, FRONT_CEN al frente y LAT a los costados.
// Ojo: el pin 1 es el TX de Serial1; no usar Serial1 con un sensor conectado ahí.
constexpr uint8_t S_FRONT_IZQ = 2;
constexpr uint8_t S_FRONT_CEN = 4;
constexpr uint8_t S_FRONT_DER = A5;
constexpr uint8_t S_LAT_IZQ   = 1;
constexpr uint8_t S_LAT_DER   = A4;

// --- Puente H de los motores ---
// Motor A = rueda izquierda, motor B = rueda derecha.
// PWMx fija la velocidad (D10 y D11 admiten PWM en el Leonardo) y MAx1/MAx2 el
// sentido de giro.
constexpr uint8_t PWMA = 10;
constexpr uint8_t MA1A = 9;
constexpr uint8_t MA2A = 13;

constexpr uint8_t PWMB = 11;
constexpr uint8_t MA1B = 8;
constexpr uint8_t MA2B = 12;

// --- Módulo de arranque (control remoto del árbitro) ---
// -1 = robot sin módulo: el combate empieza nada más encender.
constexpr int PIN_MODULO_ARRANQUE = A0;

// --- Interruptores DIP de la placa: eligen la rutina de inicio del round ---
constexpr uint8_t DIP_1 = 5;
constexpr uint8_t DIP_2 = 6;
constexpr uint8_t DIP_3 = 7;

#endif
