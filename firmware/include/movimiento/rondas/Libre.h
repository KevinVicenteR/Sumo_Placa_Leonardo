// Libre.h — Modo 4 (DIP1 ON, DIP2 ON): salida libre, arco 斜め / 回り込み
// Sale en arco a fondo abriéndose hacia el lado de DIP3 durante LibreTiempoArco
// y después gira hacia dentro (o hacia el sensor que lo vea) para atacarlo de
// lado. Cuando el frontal lo ve, ataca a fondo. Colócalo de cara al rival, con
// DIP3 hacia el lado por el que quieras rodearlo.
#ifndef LIBRE_H
#define LIBRE_H

#include "movimiento/rondas/PasoRonda.h"

// --- Velocidades (PWM 0-255) ---
constexpr int LibreVelocidadArco = 255;           // rueda exterior del arco
constexpr int LibreVelocidadArcoInterior = 110;   // rueda interior del arco
constexpr int LibreVelocidadGiro = 150;           // giro hacia dentro o hacia el sensor que lo ve
// --- Tiempos (ms) ---
// Maniobra obligatoria al empezar (sin mirar los sensores)
constexpr unsigned long LibreTiempoMinimo = 0;
constexpr unsigned long LibreTiempoArco = 200;    // duración del arco
constexpr unsigned long LibreTiempoMax = 1500;    // duración máxima de la salida

class Libre {
public:
    ResultadoRonda paso(const PasoRonda& p);
};

#endif
