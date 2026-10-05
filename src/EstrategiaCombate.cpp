#include "EstrategiaCombate.H"
#include "Parametros.H"

DecisionMovimiento EstrategiaCombate::decidir(const LecturasSensores& lecturas) const {
    DecisionMovimiento decision = elegir(lecturas);
    decision.cercaBorde = lecturas.cercaBorde;
    decision.ataqueDirecto = decision.tipo == TipoAccion::AtaqueFrontal &&
                            lecturas.c45Izq && lecturas.frontal && lecturas.c45Der;
    return decision;
}

DecisionMovimiento EstrategiaCombate::elegir(const LecturasSensores& lecturas) const {
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
