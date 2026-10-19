#ifndef MODO1_DEFENSIVO_H
#define MODO1_DEFENSIVO_H

#include "modos_rumania/PasoModo.h"

constexpr int Modo1VelocidadGiro = 200;
constexpr int Modo1VelocidadAvanceEspera = 50;
constexpr unsigned long Modo1TiempoMinimo = 0;
constexpr unsigned long Modo1TiempoQuieto = 3000;
constexpr unsigned long Modo1TiempoRecibir = 200;
constexpr unsigned long Modo1TiempoMax = 9000;

class Modo1Defensivo {
public:
    ResultadoModo paso(const PasoModo& p);

private:
    bool recibiendo = false;
    unsigned long inicioRecibir = 0;
};

#endif
