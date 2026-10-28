// Modo4Agresivo.h — Modo 4 de Rumania (DIP1 ON, DIP2 ON): de espaldas, agresivo
// Avanza recto sin verlo; si un sensor de 45° o lateral lo ve, gira hacia él a
// fondo; cuando el frontal lo ve, ataca a fondo.
// Simulador (RU4, sin el rival de 500 g): gana 56,8 % / cae 22,5 % (contragolpe 56,9 / 25,0).
#ifndef MODO4_AGRESIVO_H
#define MODO4_AGRESIVO_H

#include "movimiento/rumania/PasoModo.h"

// --- Velocidades (PWM 0-255) ---
// Recto sin verlo: no a fondo, porque a 255 el robot real no llega a frenar al
// ver la línea y se sale solo.
constexpr int Modo4VelocidadAvance = 150;
constexpr int Modo4VelocidadGiro = 255;          // giro hacia el sensor que lo ve
// --- Tiempos (ms) ---
// Maniobra obligatoria al empezar (avance recto sin mirar los sensores). 0: en
// el simulador, incluso 200 ms subían las caídas del modo 4 del 27 % al 39 %
constexpr unsigned long Modo4TiempoMinimo = 0;
constexpr unsigned long Modo4TiempoMax = 1500;   // duración máxima del modo
// Giro de 180° (docs/movimientos_rumania.md, M4): sin verlo, gira a fondo hacia
// DIP3 este tiempo en vez de avanzar recto alejándose (0 = sin giro)
constexpr unsigned long Modo4TiempoGiro180 = 0;

class Modo4Agresivo {
public:
    ResultadoModo paso(const PasoModo& p);
};

#endif
