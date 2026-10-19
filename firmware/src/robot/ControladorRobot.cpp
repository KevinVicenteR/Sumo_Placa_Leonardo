#include <Arduino.h>
#include "robot/ControladorRobot.h"

ControladorRobot::ControladorRobot(IPercepcion& percepcionRef, IEstrategiaCombate& estrategiaRef,
                                   IControlMovimiento& controlRef, IMotor& motorRef)
    : percepcion(percepcionRef), estrategia(estrategiaRef),
      controlMovimiento(controlRef), motor(motorRef) {}

void ControladorRobot::actualizar() {
    lecturas = percepcion.leer();
    DecisionMovimiento decision = estrategia.decidir(lecturas);
    controlMovimiento.ejecutar(decision, motor, millis());
}
