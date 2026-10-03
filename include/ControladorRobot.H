#ifndef CONTROLADOR_ROBOT_H
#define CONTROLADOR_ROBOT_H

#include "IMotor.H"
#include "IPercepcion.H"
#include "IEstrategiaCombate.H"
#include "IControlMovimiento.H"

// Orquesta un ciclo de control: leer sensores -> decidir -> ejecutar
class ControladorRobot {
    IPercepcion& percepcion;
    IEstrategiaCombate& estrategia;
    IControlMovimiento& controlMovimiento;
    IMotor& motor;
public:
    ControladorRobot(IPercepcion& percepcion, IEstrategiaCombate& estrategia,
                     IControlMovimiento& controlMovimiento, IMotor& motor);
    void actualizar();
};

#endif
