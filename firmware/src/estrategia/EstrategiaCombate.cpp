#include "estrategia/EstrategiaCombate.h"

DecisionMovimiento EstrategiaCombate::decidir(const LecturasSensores& lecturas) {
    DecisionMovimiento decision = elegir(lecturas);
    // Datos extra que el control de movimiento necesita
    decision.cercaBorde = lecturas.cercaBorde;
    decision.ataqueDirecto = decision.tipo == TipoAccion::AtaqueFrontal &&
                            lecturas.c45Izq && lecturas.frontal && lecturas.c45Der;
    return decision;
}

DecisionMovimiento EstrategiaCombate::elegir(const LecturasSensores& lecturas) {
    // 1. El borde primero: con línea blanca no se hace otra cosa que evadir
    if (lecturas.lineaIzq || lecturas.lineaDer) {
        if (lecturas.lineaIzq && !lecturas.lineaDer) {
            return {TipoAccion::EvadirBordeIzq, lecturas.frontal};
        }
        if (lecturas.lineaDer && !lecturas.lineaIzq) {
            return {TipoAccion::EvadirBordeDer, lecturas.frontal};
        }
        return {TipoAccion::EvadirBordeAmbos, lecturas.frontal};
    }

    // 2. Rival centrado en los tres sensores delanteros: ataque directo
    if (lecturas.c45Izq && lecturas.frontal && lecturas.c45Der) {
        return {TipoAccion::AtaqueFrontal, true};
    }

    // 3. Resto de casos: lo decide cada estrategia
    return elegirContraEnemigo(lecturas);
}
