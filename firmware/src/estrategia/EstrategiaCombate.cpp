#include "estrategia/EstrategiaCombate.h"

DecisionMovimiento EstrategiaCombate::decidir(const LecturasSensores& lecturas) {
    DecisionMovimiento decision = elegir(lecturas);
    decision.cercaBorde = lecturas.cercaBorde;
    decision.ataqueDirecto = decision.tipo == TipoAccion::AtaqueFrontal &&
                            lecturas.c45Izq && lecturas.frontal && lecturas.c45Der;
    return decision;
}

DecisionMovimiento EstrategiaCombate::elegir(const LecturasSensores& lecturas) {
    if (lecturas.lineaIzq || lecturas.lineaDer) {
        if (lecturas.lineaIzq && !lecturas.lineaDer) {
            return {TipoAccion::EvadirBordeIzq, lecturas.frontal};
        }
        if (lecturas.lineaDer && !lecturas.lineaIzq) {
            return {TipoAccion::EvadirBordeDer, lecturas.frontal};
        }
        return {TipoAccion::EvadirBordeAmbos, lecturas.frontal};
    }

    if (lecturas.c45Izq && lecturas.frontal && lecturas.c45Der) {
        return {TipoAccion::AtaqueFrontal, true};
    }

    return elegirContraEnemigo(lecturas);
}
