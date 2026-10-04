#ifndef MOTOR_H
#define MOTOR_H

#include "IMotor.H"

class Motor: public IMotor {
public:
    // Sin PWM ni freno activo: para la espera inicial y la calibración.
    void deshabilitar();
    void mover(int velIzq, int velDer) override;
    void detener() override;
};

#endif
