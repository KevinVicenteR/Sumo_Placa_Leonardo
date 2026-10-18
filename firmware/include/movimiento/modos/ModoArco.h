// ModoArco.h — Estilo arco (斜め / 回り込み): arco a fondo hacia DIP3 y giro hacia dentro
// Sale en arco a fondo abriéndose hacia el lado de DIP3 durante ArcoTiempo y
// después gira hacia dentro (o hacia el sensor que lo vea) para atacarlo de
// lado. Cuando el frontal lo ve, ataca a fondo. Es el modo 4 (salida libre) de
// pesos_neuronales_rutina3: colócalo de cara al rival, con DIP3 hacia el lado
// por el que quieras rodearlo.
#ifndef MODO_ARCO_H
#define MODO_ARCO_H

#include "movimiento/modos/PasoModo.h"

// --- Velocidades (PWM 0-255) ---
constexpr int ArcoVelocidad = 255;               // rueda exterior del arco
constexpr int ArcoVelocidadInterior = 110;       // rueda interior del arco
constexpr int ArcoVelocidadGiro = 150;           // giro hacia dentro o hacia el sensor que lo ve
// --- Tiempos (ms) ---
// Maniobra obligatoria al empezar (sin mirar los sensores)
constexpr unsigned long ArcoTiempoMinimo = 0;
constexpr unsigned long ArcoTiempo = 200;        // duración del arco
constexpr unsigned long ArcoTiempoMax = 1500;    // duración máxima del modo

class ModoArco {
public:
    ResultadoModo paso(const PasoModo& p);
};

#endif
