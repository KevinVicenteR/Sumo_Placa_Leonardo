#include "movimiento/rumania/Modo4Agresivo.h"

ResultadoModo Modo4Agresivo::paso(const PasoModo& p) {
    if (p.deFrente) {
        return ResultadoModo::Ataca;                          // lo ve de frente: a fondo
    }
    if (p.t >= Modo4TiempoMax) {
        return ResultadoModo::Termina;
    }
    if (p.visto == 0 && p.t < Modo4TiempoGiro180) {
        p.mando.mover(p.motor, p.lado * Modo4VelocidadGiro, -p.lado * Modo4VelocidadGiro, true);
    } else if (p.visto == 0) {
        p.mando.mover(p.motor, Modo4VelocidadAvance, Modo4VelocidadAvance, true);   // recto
    } else {
        // Lo ve un sensor de 45° o lateral: giro a fondo hacia él
        p.mando.mover(p.motor, p.visto * Modo4VelocidadGiro, -p.visto * Modo4VelocidadGiro, true);
    }
    return ResultadoModo::Sigue;
}
