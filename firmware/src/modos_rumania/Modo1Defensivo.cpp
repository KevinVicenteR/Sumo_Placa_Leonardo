#include "modos_rumania/Modo1Defensivo.h"

ResultadoModo Modo1Defensivo::paso(const PasoModo& p) {
    if (!p.deFrente) {
        recibiendo = false;
    } else {
        if (!recibiendo) {
            recibiendo = true;
            inicioRecibir = p.ahora;
        }
        if (p.ahora - inicioRecibir < Modo1TiempoRecibir) {
            p.mando.mover(p.motor, 0, 0, true);
            return ResultadoModo::Sigue;
        }
        return ResultadoModo::Ataca;
    }
    if (p.t >= Modo1TiempoMax) {
        return ResultadoModo::Termina;
    }
    if (p.visto == 0) {
        const int vel = p.t < Modo1TiempoQuieto ? 0 : Modo1VelocidadAvanceEspera;
        p.mando.mover(p.motor, vel, vel, true);
        return ResultadoModo::Sigue;
    }
    p.mando.mover(p.motor, p.visto * Modo1VelocidadGiro, -p.visto * Modo1VelocidadGiro, true);
    return ResultadoModo::Sigue;
}
