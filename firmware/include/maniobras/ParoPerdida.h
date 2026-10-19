#ifndef PARO_PERDIDA_H
#define PARO_PERDIDA_H

#include "movimiento/MandoMotores.h"
#include "estrategia/DecisionMovimiento.h"

class ParoPerdida {
public:
    bool continuar(const DecisionMovimiento& decision, MandoMotores& mando, IMotor& motor,
                   unsigned long ahora);

    void cancelar() { frenando = false; }
    void olvidar() { frenando = false; veniaAtacando = false; }
    void marcarAtaque() { frenando = false; veniaAtacando = true; }

private:
    bool veniaAtacando = false;
    bool frenando = false;
    unsigned long inicio = 0;
    Freno freno;
};

#endif
