#ifndef DRIVER_MOTORES_H
#define DRIVER_MOTORES_H

#include <stdint.h>
#include "hardware/IMotor.h"

class DriverMotores: public IMotor {
public:
    void mover(int velIzq, int velDer) override;
    void detener() override;
    void deshabilitar();

private:
    static void controlarMotor(uint8_t in1, uint8_t in2, uint8_t pwm, int velocidad);
};

#endif
