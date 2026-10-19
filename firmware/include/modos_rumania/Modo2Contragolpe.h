#ifndef MODO2_CONTRAGOLPE_H
#define MODO2_CONTRAGOLPE_H

#include "modos_rumania/PasoModo.h"

constexpr int Modo2VelocidadGiro = 150;
constexpr int Modo2VelocidadAvanceEspera = 100;
constexpr unsigned long Modo2TiempoMinimo = 300;
constexpr unsigned long Modo2TiempoQuieto = 3000;
constexpr unsigned long Modo2TiempoMax = 6000;

class Modo2Contragolpe {
public:
    ResultadoModo paso(const PasoModo& p);
};

#endif
