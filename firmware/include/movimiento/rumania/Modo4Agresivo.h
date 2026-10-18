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
constexpr unsigned long Modo4TiempoMax = 1500;   // duración máxima del modo

class Modo4Agresivo {
public:
    ResultadoModo paso(const PasoModo& p);
};

#endif
