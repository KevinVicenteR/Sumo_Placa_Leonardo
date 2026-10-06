// IControlMovimiento.h — Interfaz del control de movimiento
// Recibe la decisión de la estrategia y mueve los motores en consecuencia.
#ifndef ICONTROL_MOVIMIENTO_H
#define ICONTROL_MOVIMIENTO_H

#include "hardware/IMotor.h"
#include "estrategia/DecisionMovimiento.h"

class IControlMovimiento {
public:
    virtual ~IControlMovimiento() {}
    virtual void ejecutar(const DecisionMovimiento& decision, IMotor& motor, unsigned long ahora) = 0;
};

#endif
