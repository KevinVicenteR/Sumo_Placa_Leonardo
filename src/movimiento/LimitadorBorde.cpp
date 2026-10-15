#include "movimiento/LimitadorBorde.h"
#include "Parametros.h"

namespace {
constexpr long metros(float m) { return EstimadorBorde::metros(m); }
constexpr long radianes(float r) { return EstimadorBorde::radianes(r); }
}

void LimitadorBorde::colocarSalida(int rutina, int lado) {
    if (rutina == 1 || rutina == 2) {
        // Espalda con espalda o lado a lado: junto al centro, rumbo cualquiera
        estimadorBorde.colocarQ(0, metros(IncertSalidaCentro), false, 0);
    } else if (rutina == 3) {
        // Enfrentados: junto al borde, mirando al centro
        estimadorBorde.colocarQ(metros(RadioSalidaRound3), metros(IncertSalidaRound3), true,
                                radianes(IncertRumboRound3));
    } else if (rutina >= 5 && rutina <= 8) {
        // Colocación libre (lo coloca uno mismo, con cuidado): de frente (5 y 8),
        // girado 45° (6) o de lado (7), con el rival hacia el lado de DIP3
        const long desvio = rutina == 6 ? EstimadorBorde::Vuelta / 8 : rutina == 7 ? EstimadorBorde::Vuelta / 4 : 0;
        estimadorBorde.colocarGiradoQ(metros(RadioSalidaRound3), metros(IncertSalidaLibre),
                                      lado < 0 ? -desvio : desvio, radianes(IncertRumboLibre));
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
        long empujon = 0;
        if (empujando) {
            empujon = metros(EmpujonContacto);
        } else if (esAtaque(decision.tipo)) {
            empujon = metros(EmpujonAtaque);
        } else if (esDefensa(decision.tipo)) {
            empujon = metros(EmpujonLateral);
        }
        if (dtMs < 1000) {
            estimadorBorde.predecirMs(mando.ordenIzquierda(), mando.ordenDerecha(), (unsigned int)dtMs, empujon);
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
