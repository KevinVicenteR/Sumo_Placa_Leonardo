// =============================================================================
// DriverMotores.h — Control del puente H de las dos ruedas
// -----------------------------------------------------------------------------
// Implementación real de IMotor: traduce una velocidad con signo a los pines de
// sentido (MAx1/MAx2) y de potencia (PWMx) de cada motor.
// =============================================================================
#ifndef DRIVER_MOTORES_H
#define DRIVER_MOTORES_H

#include <stdint.h>
#include "hardware/IMotor.h"

class DriverMotores: public IMotor {
public:
    void mover(int velIzq, int velDer) override;
    void detener() override;
    // Deja los motores sin potencia ni freno: para la espera y la calibración
    void deshabilitar();

private:
    static void controlarMotor(uint8_t in1, uint8_t in2, uint8_t pwm, int velocidad);
};

#endif
