// DriverMotores.h — Control de los dos drivers IFX9201SG de las ruedas
// Implementación real de IMotor: traduce una velocidad con signo al pin de
// sentido (DIR) y al de potencia (PWM) de cada motor.
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
    static void controlarMotor(uint8_t dir, uint8_t pwm, int velocidad);
};

#endif
