#include "movimiento/LimitadorBorde.h"
#include "Parametros.h"

void LimitadorBorde::colocarSalida(int rutina, int lado) {
    if (ColocacionLibre) {
        if (EstimarSalidaRumania && rutina >= 1 && rutina <= 4) {
            constexpr float Pi = 3.14159265f;
            // Centro detrás del rival: 1-2 lo miran, 3 lo tiene al lado (DIP3), 4 de espaldas
            const float rumbo = rutina <= 2 ? Pi : rutina == 3 ? Pi + lado * Pi / 2 : 0;
            estimadorBorde.colocarRumbo(RadioSalidaRumania, IncertSalidaRumania, rumbo, IncertRumboRumania);
        }
        return;
    }
    if (rutina == 1 || rutina == 2) {
        // Espalda con espalda o lado a lado: junto al centro, rumbo cualquiera
        estimadorBorde.colocar(0, IncertSalidaCentro, false, 0);
    } else if (rutina == 3) {
        // Enfrentados: junto al borde, mirando al centro
        estimadorBorde.colocar(RadioSalidaRound3, IncertSalidaRound3, true, IncertRumboRound3);
    }
}

void LimitadorBorde::actualizar(const DecisionMovimiento& decision, const MandoMotores& mando,
                                bool empujando, unsigned long ahora) {
    // 1. Predecir el movimiento desde el último ciclo con las órdenes enviadas
    const unsigned long dtMs = ahora - ultimaPrediccion;
    bool cambio = dtMs > 0;
    if (dtMs > 0) {
        ultimaPrediccion = ahora;
        // Con el rival encima, el robot se mueve sin que el estimador lo sepa
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

    // 2. Línea recién vista: corrige la posición estimada
    const bool borde = esBorde(decision.tipo);
    if (borde && !bordeAntes) {
        const int lado = decision.tipo == TipoAccion::EvadirBordeIzq ? -1
                       : decision.tipo == TipoAccion::EvadirBordeDer ? 1 : 0;
        estimadorBorde.lineaVista(lado, mando.avanzando());
        cambio = true;
    }
    bordeAntes = borde;

    // 3. Nuevo límite (solo si la predicción es fiable)
    if (cambio) {
        limiteAvance = estimadorBorde.confiable() ? estimadorBorde.limiteAvance() : 255;
    }
}
