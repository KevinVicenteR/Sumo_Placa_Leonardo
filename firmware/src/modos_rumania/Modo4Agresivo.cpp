#include "modos_rumania/Modo4Agresivo.h"

ResultadoModo Modo4Agresivo::paso(const PasoModo& p) {
    if (p.deFrente) {
        return ResultadoModo::Ataca;
    }
    if (p.t >= Modo4TiempoMax) {
        return ResultadoModo::Termina;
    }
    if (p.visto == 0) {
        p.mando.mover(p.motor, Modo4VelocidadAvance, Modo4VelocidadAvance, true);
    } else {
        p.mando.mover(p.motor, p.visto * Modo4VelocidadGiro, -p.visto * Modo4VelocidadGiro, true);
    }
    return ResultadoModo::Sigue;
}
