#include "estrategia/EstrategiaReglas.h"

DecisionMovimiento EstrategiaReglas::elegirContraEnemigo(const LecturasSensores& lecturas) {
    if (lecturas.frontal) {
        if (lecturas.c45Izq && !lecturas.c45Der) {
            return {TipoAccion::AjusteIzq, true};
        }
        if (lecturas.c45Der && !lecturas.c45Izq) {
            return {TipoAccion::AjusteDer, true};
        }
        return {TipoAccion::AtaqueFrontal, true};
    }

    if (lecturas.c45Izq) {
        return {TipoAccion::CorregirIzq};
    }
    if (lecturas.c45Der) {
        return {TipoAccion::CorregirDer};
    }

    if (lecturas.latIzq) {
        return {TipoAccion::DefensaIzq};
    }
    if (lecturas.latDer) {
        return {TipoAccion::DefensaDer};
    }

    return {TipoAccion::Busqueda};
}
