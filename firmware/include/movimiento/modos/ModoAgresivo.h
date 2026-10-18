// ModoAgresivo.h — Estilo agresivo: avance recto y giro a fondo hacia el rival (en qué modo se usa: PasoModo.h)
// Avanza recto sin verlo; si un sensor de 45° o lateral lo ve, gira hacia él a
// fondo; cuando el frontal lo ve, ataca a fondo.
// Simulador (RU4, sin el rival de 500 g): gana 56,8 % / cae 22,5 % (contragolpe 56,9 / 25,0).
#ifndef MODO_AGRESIVO_H
#define MODO_AGRESIVO_H

#include "movimiento/modos/PasoModo.h"

// --- Velocidades (PWM 0-255) ---
// Recto sin verlo: no a fondo, porque a 255 el robot real no llega a frenar al
// ver la línea y se sale solo.
constexpr int AgresivoVelocidadAvance = 150;
constexpr int AgresivoVelocidadGiro = 255;          // giro hacia el sensor que lo ve
// --- Tiempos (ms) ---
// Maniobra obligatoria al empezar (avance recto sin mirar los sensores). 0: en
// el simulador, incluso 200 ms subían las caídas del modo 4 del 27 % al 39 %
constexpr unsigned long AgresivoTiempoMinimo = 0;
constexpr unsigned long AgresivoTiempoMax = 1500;   // duración máxima del modo

class ModoAgresivo {
public:
    ResultadoModo paso(const PasoModo& p);
};

#endif
