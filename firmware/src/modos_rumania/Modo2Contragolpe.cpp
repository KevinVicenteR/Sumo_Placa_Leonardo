#include "modos_rumania/Modo2Contragolpe.h"

ResultadoModo Modo2Contragolpe::paso(const PasoModo& p) {
    if (p.deFrente) {
        return ResultadoModo::Ataca;
    }
    if (p.t >= Modo2TiempoMax) {
        return ResultadoModo::Termina;
    }
    if (p.visto == 0) {
        const int vel = p.t < Modo2TiempoQuieto ? 0 : Modo2VelocidadAvanceEspera;
        p.mando.mover(p.motor, vel, vel, true);
        return ResultadoModo::Sigue;
    }
    p.mando.mover(p.motor, p.visto * Modo2VelocidadGiro, -p.visto * Modo2VelocidadGiro, true);
    return ResultadoModo::Sigue;
}
