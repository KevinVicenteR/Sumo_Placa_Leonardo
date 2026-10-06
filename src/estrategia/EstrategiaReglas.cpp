#include "estrategia/EstrategiaReglas.h"

DecisionMovimiento EstrategiaReglas::elegirContraEnemigo(const LecturasSensores& lecturas) {
    // Rival de frente: atacar, corrigiendo hacia el lado de un sensor de 45°
    if (lecturas.frontal) {
        if (lecturas.c45Izq && !lecturas.c45Der) {
            return {TipoAccion::AjusteIzq, true};
        }
        if (lecturas.c45Der && !lecturas.c45Izq) {
            return {TipoAccion::AjusteDer, true};
        }
        return {TipoAccion::AtaqueFrontal, true};
    }

    // Rival solo a 45°: girar hacia él mientras avanza
    if (lecturas.c45Izq) {
        return {TipoAccion::CorregirIzq};
    }
    if (lecturas.c45Der) {
        return {TipoAccion::CorregirDer};
    }

    // Rival de lado: girar para ponerlo de frente
    if (lecturas.latIzq) {
        return {TipoAccion::DefensaIzq};
    }
    if (lecturas.latDer) {
        return {TipoAccion::DefensaDer};
    }

    // No se ve: buscar
    return {TipoAccion::Busqueda};
}
