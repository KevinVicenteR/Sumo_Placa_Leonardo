// ControladorRobot.h — El ciclo principal: percibir -> decidir -> actuar
// No sabe nada del hardware ni de la estrategia concreta: recibe sus piezas
// por el constructor (inyección de dependencias) y solo las encadena.
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
    // Un ciclo completo de combate
    void actualizar();

private:
    IPercepcion& percepcion;
    IEstrategiaCombate& estrategia;
    IControlMovimiento& controlMovimiento;
    IMotor& motor;
};

#endif
