#ifndef MOTOR_H
#define MOTOR_H

#include "IMotor.H"

class Motor: public IMotor {
public:
    void deshabilitar();
    void mover(int velIzq, int velDer) override;
    void detener() override;
};

#endif
