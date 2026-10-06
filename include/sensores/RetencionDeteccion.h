// =============================================================================
// RetencionDeteccion.h — Filtro de un sensor de enemigo
// -----------------------------------------------------------------------------
// - Una detección nueva solo se acepta si dura confirmacionMs seguidos
//   (descarta reflejos y lecturas sueltas).
// - Una vez aceptada, se mantiene duracionMs después de perderla (tolera los
//   parpadeos del sensor).
// =============================================================================
#ifndef RETENCION_DETECCION_H
#define RETENCION_DETECCION_H

class RetencionDeteccion {
public:
    explicit RetencionDeteccion(unsigned long duracionMs, unsigned long confirmacionMs = 0)
        : duracionMs(duracionMs), confirmacionMs(confirmacionMs) {}

    // Recibe la lectura cruda y devuelve la lectura filtrada
    bool actualizar(bool detectado, unsigned long ahora) {
        if (detectado) {
            // Empieza (o sigue) una posible detección
            if (!candidato) {
                candidato = true;
                inicioCandidato = ahora;
            }
            if (activo || ahora - inicioCandidato >= confirmacionMs) {
                ultimaDeteccion = ahora;
                activo = true;
                return true;
            }
            return false;
        }
        // Sin lectura: se mantiene la detección mientras dure la retención
        candidato = false;
        if (activo && ahora - ultimaDeteccion < duracionMs) {
            return true;
        }
        activo = false;
        return false;
    }

private:
    unsigned long duracionMs;
    unsigned long confirmacionMs;
    unsigned long ultimaDeteccion = 0;
    unsigned long inicioCandidato = 0;
    bool activo = false;
    bool candidato = false;
};

#endif
