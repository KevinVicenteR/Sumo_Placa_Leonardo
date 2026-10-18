// Ronda1.h — Modo 1 (DIP1 off, DIP2 off): espalda con espalda
// Media vuelta en el sitio hacia el lado de DIP3 hasta ver al rival de frente;
// al verlo frena el giro y lo embiste (VelocidadEmbestida* en ParametrosAtaque.h).
#ifndef RONDA1_H
#define RONDA1_H

#include "movimiento/rondas/PasoRonda.h"
#include "movimiento/Confirmacion.h"

// --- Velocidades (PWM 0-255) ---
constexpr int Ronda1VelocidadGiro = 180;          // media vuelta en el sitio
constexpr int Ronda1VelocidadEsquiva = 160;       // avance recto antes de girar
// --- Tiempos (ms) ---
// Antes de girar, avanza recto este tiempo para separarse del rival (0 = gira
// enseguida): están pegados y, con las ruedas atrás, al girar las esquinas
// traseras barren unos 3,6 cm hacia atrás, chocan con él y cortan el giro.
constexpr unsigned long Ronda1TiempoEsquiva = 0;
constexpr unsigned long Ronda1TiempoMinimoGiro = 80;   // una detección no termina el giro antes
constexpr unsigned long Ronda1TiempoMaxGiro = 500;     // duración máxima del giro

class Ronda1 {
public:
    ResultadoRonda paso(const PasoRonda& p);

private:
    Confirmacion rivalVisto;
    bool frenando = false;
    unsigned long inicioFreno = 0;
    Freno freno;
};

#endif
