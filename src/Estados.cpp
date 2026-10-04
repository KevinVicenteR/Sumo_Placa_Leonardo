#include <Arduino.h>
#include "Estados.H"

Estado::Estado(IPercepcion& percepcionRef, IEstrategiaCombate& estrategiaRef, IControlMovimiento& controlRef)
    : motor(nullptr),
            percepcion(percepcionRef), estrategia(estrategiaRef), controlMovimiento(controlRef) {}

void Estado::setMotor(IMotor* nuevoMotor) {
    motor = nuevoMotor;
}

void Estado::actualizarEstado() {
    if (motor == nullptr) {
        return;
    }

    LecturasSensores lecturas = percepcion.leer();
    if (lecturas.lineaIzq || lecturas.lineaDer) {
        const TipoAccion accion = lecturas.lineaIzq && lecturas.lineaDer ? TipoAccion::EvadirBordeAmbos :
                                 lecturas.lineaIzq ? TipoAccion::EvadirBordeIzq : TipoAccion::EvadirBordeDer;
        controlMovimiento.ejecutar({accion}, *motor);
        return;
    }
    DecisionMovimiento decision = estrategia.decidir(lecturas);
    // El borde puede aparecer mientras se leen enemigos o se decide el ataque.
    // Revalidar el piso antes de enviar cualquier orden normal al motor.
    const LecturasSensores borde = percepcion.leerBorde();
    if (borde.lineaIzq || borde.lineaDer) {
        decision = {borde.lineaIzq && borde.lineaDer ? TipoAccion::EvadirBordeAmbos :
                    borde.lineaIzq ? TipoAccion::EvadirBordeIzq : TipoAccion::EvadirBordeDer};
    }
    controlMovimiento.ejecutar(decision, *motor);
}
