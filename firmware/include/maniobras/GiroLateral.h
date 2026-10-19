#ifndef GIRO_LATERAL_H
#define GIRO_LATERAL_H

#include "movimiento/MandoMotores.h"
#include "movimiento/Confirmacion.h"
#include "estrategia/DecisionMovimiento.h"

class GiroLateral {
public:
    bool continuar(const DecisionMovimiento& decision, MandoMotores& mando, IMotor& motor,
                   unsigned long ahora);
    void cancelar() { sentido = 0; }

    static void girarHacia(MandoMotores& mando, IMotor& motor, int sentido);

private:
    int sentido = 0;
    unsigned long inicio = 0;
    Confirmacion rivalDelante;
};

#endif
