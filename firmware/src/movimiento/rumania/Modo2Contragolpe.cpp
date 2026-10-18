#include "movimiento/rumania/Modo2Contragolpe.h"

ResultadoModo Modo2Contragolpe::paso(const PasoModo& p) {
    if (p.deFrente) {
        return ResultadoModo::Ataca;                          // lo ve de frente: a fondo
    }
    if (p.t >= Modo2TiempoMax) {
        return ResultadoModo::Termina;
    }
    if (p.visto == 0) {
        // Quieto un rato esperándolo; si no aparece, avanza despacio a buscarlo
        const int vel = p.t < Modo2TiempoQuieto ? 0 : Modo2VelocidadAvanceEspera;
        p.mando.mover(p.motor, vel, vel, true);
        return ResultadoModo::Sigue;
    }
    // Lo ve un sensor de 45° o lateral: se encara girando hacia él
    p.mando.mover(p.motor, p.visto * Modo2VelocidadGiro, -p.visto * Modo2VelocidadGiro, true);
    return ResultadoModo::Sigue;
}
