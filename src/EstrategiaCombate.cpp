#include "EstrategiaCombate.H"

DecisionMovimiento EstrategiaCombate::decidir(const LecturasSensores& lecturas) const {
    if (lecturas.lineaIzq || lecturas.lineaDer) {
        if (lecturas.lineaIzq && !lecturas.lineaDer) {
            return {TipoAccion::EvadirBordeIzq, lecturas.frontal};
        }
        if (lecturas.lineaDer && !lecturas.lineaIzq) {
            return {TipoAccion::EvadirBordeDer, lecturas.frontal};
        }
        return {TipoAccion::EvadirBordeAmbos, lecturas.frontal};
    }

    if (lecturas.frontal) {
        // Con un solo sensor de 45° también activo, el enemigo está algo hacia
        // ese lado: se ataca corrigiendo un poco para pegarle de frente
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
