#include "estrategia/EstrategiaCombate.h"
#include "Parametros.h"

DecisionMovimiento EstrategiaCombate::decidir(const LecturasSensores& lecturas) {
    DecisionMovimiento decision = elegir(lecturas);
    // Datos extra que el control de movimiento necesita
    decision.cercaBorde = lecturas.cercaBorde;
    decision.ataqueDirecto = decision.tipo == TipoAccion::AtaqueFrontal &&
                            lecturas.c45Izq && lecturas.frontal && lecturas.c45Der;
    decision.errorRival = errorRival(lecturas);
    return decision;
}

int EstrategiaCombate::errorRival(const LecturasSensores& lecturas) {
    const int vistos = lecturas.c45Izq + lecturas.frontal + lecturas.c45Der;
    if (vistos == 0) return 0;
    return (lecturas.c45Der - lecturas.c45Izq) * PosicionSensor45PID / vistos;
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
        if (!priorizarAtaqueFrontal()) {
            DecisionMovimiento decision = elegirContraEnemigo(lecturas);
            // Solo una esquiva deliberada sustituye al ataque directo. No
            // convertir una lectura centrada en una corrección lateral espuria.
            if (!esEsquiva(decision.tipo)) decision.tipo = TipoAccion::AtaqueFrontal;
            return decision;
        }
        return {TipoAccion::AtaqueFrontal, true};
    }

    // 3. Resto de casos: lo decide cada estrategia
    return elegirContraEnemigo(lecturas);
}
