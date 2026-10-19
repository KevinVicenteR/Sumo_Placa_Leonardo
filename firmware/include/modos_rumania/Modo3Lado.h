#ifndef MODO3_LADO_H
#define MODO3_LADO_H

#include "modos_rumania/PasoModo.h"

constexpr int Modo3VelocidadGiro = 150;
constexpr int Modo3VelocidadBarrido = 120;
constexpr unsigned long Modo3TiempoMinimo = 0;
constexpr unsigned long Modo3TiempoGiro = 500;
constexpr unsigned long Modo3TiempoMax = 1500;

class Modo3Lado {
public:
    ResultadoModo paso(const PasoModo& p);

private:
    int sentido = 0;
};

#endif
