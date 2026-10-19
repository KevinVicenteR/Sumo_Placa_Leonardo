#ifndef PASO_MODO_H
#define PASO_MODO_H

#include "movimiento/MandoMotores.h"
#include "estrategia/DecisionMovimiento.h"

struct PasoModo {
    const DecisionMovimiento& decision;
    MandoMotores& mando;
    IMotor& motor;
    unsigned long ahora;
    unsigned long t;
    int lado;
    int visto;
    bool deFrente;
};

enum class ResultadoModo : unsigned char {
    Sigue,
    Ataca,
    Termina
};

#endif
