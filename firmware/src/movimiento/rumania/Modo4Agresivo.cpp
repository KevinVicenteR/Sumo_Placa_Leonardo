#include "movimiento/rumania/Modo4Agresivo.h"

ResultadoModo Modo4Agresivo::paso(const PasoModo& p) {
    if (p.deFrente) {
        return ResultadoModo::Ataca;                          // lo ve de frente: a fondo
    }
    if (p.t >= Modo4TiempoMax) {
        return ResultadoModo::Termina;
    }
    if (Modo4EstabilizarFrontal && p.t >= Modo4TiempoMinimo && p.decision.enemigoFrente) {
        // La detección todavía se está confirmando: deja de girar para que
        // la inercia no saque al rival del frontal antes de poder atacar.
        p.mando.mover(p.motor, Modo4VelocidadConfirmacion, Modo4VelocidadConfirmacion, true);
        return ResultadoModo::Sigue;
    }
    if (p.visto == 0) {
        p.mando.mover(p.motor, Modo4VelocidadAvance, Modo4VelocidadAvance, true);   // recto
    } else {
        // Lo ve un sensor de 45° o lateral: giro a fondo hacia él
        p.mando.mover(p.motor, p.visto * Modo4VelocidadGiro, -p.visto * Modo4VelocidadGiro, true);
    }
    return ResultadoModo::Sigue;
}
