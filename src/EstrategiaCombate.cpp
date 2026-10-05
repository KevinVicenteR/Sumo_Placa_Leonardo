#include "EstrategiaCombate.H"
#include "Parametros.H"
#include "PoliticaAprendida.H"

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

    // Esta detección prevalece también sobre la política del simulador.
    if (lecturas.c45Izq && lecturas.frontal && lecturas.c45Der) {
        return {TipoAccion::AtaqueFrontal, true};
    }

    if (UsarPoliticaAprendida) {
        return elegirConPolitica(lecturas);
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

// Sin línea, la maniobra la decide la tabla aprendida en el simulador
DecisionMovimiento EstrategiaCombate::elegirConPolitica(const LecturasSensores& lecturas) const {
    const int estado = politica::codificarEstado(lecturas, ladoUltimo);
    if (lecturas.c45Izq || lecturas.latIzq) {
        ladoUltimo = 0;
    } else if (lecturas.c45Der || lecturas.latDer) {
        ladoUltimo = 1;
    }
    return {politica::Acciones[politica::elegirAccion(estado)], lecturas.frontal};
}
