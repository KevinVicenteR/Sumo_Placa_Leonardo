#ifndef MODO4_AGRESIVO_H
#define MODO4_AGRESIVO_H

#include "modos_rumania/PasoModo.h"

constexpr int Modo4VelocidadAvance = 150;
constexpr int Modo4VelocidadGiro = 255;
constexpr unsigned long Modo4TiempoMinimo = 0;
constexpr unsigned long Modo4TiempoMax = 1500;

class Modo4Agresivo {
public:
    ResultadoModo paso(const PasoModo& p);
};

#endif
