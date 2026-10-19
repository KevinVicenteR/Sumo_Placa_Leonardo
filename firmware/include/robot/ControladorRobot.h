#ifndef CONTROLADOR_ROBOT_H
#define CONTROLADOR_ROBOT_H

#include "hardware/IMotor.h"
#include "sensores/IPercepcion.h"
#include "estrategia/IEstrategiaCombate.h"
#include "movimiento/IControlMovimiento.h"

class ControladorRobot {
public:
    ControladorRobot(IPercepcion& percepcion, IEstrategiaCombate& estrategia,
                     IControlMovimiento& controlMovimiento, IMotor& motor);
    void actualizar();
    const LecturasSensores& ultimasLecturas() const { return lecturas; }

private:
    IPercepcion& percepcion;
    IEstrategiaCombate& estrategia;
    IControlMovimiento& controlMovimiento;
    IMotor& motor;
    LecturasSensores lecturas{};
};

#endif
