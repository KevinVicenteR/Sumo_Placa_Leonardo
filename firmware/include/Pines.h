// Pines.h — Mapa de conexiones del mini sumo Boldenona (Arduino Leonardo)
// Único lugar donde se dice qué está conectado a cada pin. Si se cambia un
// cable de sitio, solo hay que tocar este archivo.
#ifndef PINES_H
#define PINES_H

#include <Arduino.h>

// --- Sensores de piso QTR Micro (analógicos): ven la línea blanca del borde ---
// Van al frente, justo después de la rampa.
constexpr uint8_t S_PISO_IZQ  = A0;
constexpr uint8_t S_PISO_DER  = A1;

// --- Sensores de enemigo JS40F (digitales): detectan al robot rival ---
// FRONT_IZQ y FRONT_DER miran a 45°, FRONT_CEN al frente y LAT a los costados.
// Ojo: el pin 1 es el TX de Serial1; no usar Serial1 con un sensor conectado ahí.
constexpr uint8_t S_FRONT_IZQ = 2;
constexpr uint8_t S_FRONT_CEN = 4;
constexpr uint8_t S_FRONT_DER = A5;
constexpr uint8_t S_LAT_IZQ   = 1;
constexpr uint8_t S_LAT_DER   = A4;

// --- Driver IFX9201SG de cada motor (JSUMO 700 rpm) ---
// PWM fija la velocidad (D10 y D11 admiten PWM en el Leonardo) y DIR el
// sentido de giro.
constexpr uint8_t PWM_IZQ = 10;
constexpr uint8_t DIR_IZQ = 13;

constexpr uint8_t PWM_DER = 11;
constexpr uint8_t DIR_DER = 12;

// --- Módulo de arranque START-STOP (Nanolabs / Robomaniacs) ---
// Provisional en A2 (A0 es el piso izquierdo). -1 = sin módulo: arranca al encender.
constexpr int PIN_MODULO_ARRANQUE = A2;

// --- Interruptores DIP: eligen la rutina de inicio del round ---
constexpr uint8_t DIP_1 = 5;
constexpr uint8_t DIP_2 = 6;
constexpr uint8_t DIP_3 = 7;

#endif
