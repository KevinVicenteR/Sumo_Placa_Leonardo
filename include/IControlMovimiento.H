#ifndef ICONTROL_MOVIMIENTO_H
#define ICONTROL_MOVIMIENTO_H

#include "IMotor.H"
#include "DecisionMovimiento.H"

class IControlMovimiento {
public:
    virtual ~IControlMovimiento() {}
    // No bloqueante: se llama en cada ciclo con el tiempo actual en ms
    virtual void ejecutar(const DecisionMovimiento& decision, IMotor& motor, unsigned long ahora) = 0;
};

#endif
