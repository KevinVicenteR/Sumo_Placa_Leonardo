#include "borde/LimitadorBorde.h"
#include "configuracion/Parametros.h"

void LimitadorBorde::colocarSalida(int rutina) {
    if (ColocacionLibre) {
        return;
    }
    if (rutina == 1 || rutina == 2) {
        estimadorBorde.colocar(0, IncertSalidaCentro, false, 0);
    } else if (rutina == 3) {
        estimadorBorde.colocar(RadioSalidaRound3, IncertSalidaRound3, true, IncertRumboRound3);
    }
}

void LimitadorBorde::actualizar(const DecisionMovimiento& decision, const MandoMotores& mando,
                                bool empujando, unsigned long ahora) {
    const unsigned long dtMs = ahora - ultimaPrediccion;
    bool cambio = dtMs > 0;
    if (dtMs > 0) {
        ultimaPrediccion = ahora;
        float empujon = 0;
        if (empujando) {
            empujon = EmpujonContacto;
        } else if (esAtaque(decision.tipo)) {
            empujon = EmpujonAtaque;
        } else if (esDefensa(decision.tipo)) {
            empujon = EmpujonLateral;
        }
        if (dtMs < 1000) {
            estimadorBorde.predecir(mando.ordenIzquierda(), mando.ordenDerecha(), dtMs / 1000.0f, empujon);
        }
    }

    const bool borde = esBorde(decision.tipo);
    if (borde && !bordeAntes) {
        const int lado = decision.tipo == TipoAccion::EvadirBordeIzq ? -1
                       : decision.tipo == TipoAccion::EvadirBordeDer ? 1 : 0;
        estimadorBorde.lineaVista(lado, mando.avanzando());
        cambio = true;
    }
    bordeAntes = borde;

    if (cambio) {
        limiteAvance = estimadorBorde.confiable() ? estimadorBorde.limiteAvance() : 255;
    }
}
