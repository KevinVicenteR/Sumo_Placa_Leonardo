// Ronda3.h — Modo 3 (DIP1 off, DIP2 ON): enfrentados, contragolpe
// Quieto hasta verlo: si un sensor de 45° o lateral lo ve, gira hacia él; cuando
// el frontal lo ve, ataca a fondo al instante. Si no aparece en
// Ronda3TiempoQuieto, avanza despacio a buscarlo.
// Simulador (R3 enfrentados, sin el rival de 500 g): gana 53,2 % / cae 35,0 %
// (el avance lento de antes: 65,0 / 24,1; sin la espera inicial,
// Ronda3TiempoMinimo y Ronda3TiempoQuieto a 0: 62,4 / 25,7).
#ifndef RONDA3_H
#define RONDA3_H

#include "movimiento/rondas/PasoRonda.h"

// --- Velocidades (PWM 0-255) ---
constexpr int Ronda3VelocidadGiro = 150;          // giro hacia el sensor que lo ve
constexpr int Ronda3VelocidadAvanceEspera = 100;  // avance lento si no aparece
// --- Tiempos (ms) ---
// Siempre hace primero su maniobra (quieto) durante este tiempo, aunque los
// sensores vean algo; solo la línea del borde la interrumpe
constexpr unsigned long Ronda3TiempoMinimo = 300;
constexpr unsigned long Ronda3TiempoQuieto = 3000;    // quieto antes de avanzar
constexpr unsigned long Ronda3TiempoMax = 6000;      // duración máxima de la ronda

class Ronda3 {
public:
    ResultadoRonda paso(const PasoRonda& p);
};

#endif
