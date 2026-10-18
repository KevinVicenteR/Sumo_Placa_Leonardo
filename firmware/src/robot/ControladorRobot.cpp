#include <Arduino.h>
#include "robot/ControladorRobot.h"

ControladorRobot::ControladorRobot(IPercepcion& percepcionRef, IEstrategiaCombate& estrategiaRef,
                                   IControlMovimiento& controlRef, IMotor& motorRef)
    : percepcion(percepcionRef), estrategia(estrategiaRef),
      controlMovimiento(controlRef), motor(motorRef) {}

void ControladorRobot::actualizar() {
    lecturas = percepcion.leer();                                 // 1. percibir
    DecisionMovimiento decision = estrategia.decidir(lecturas);   // 2. decidir
    controlMovimiento.ejecutar(decision, motor, millis());        // 3. actuar
}
