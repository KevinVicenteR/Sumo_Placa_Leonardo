#include "movimiento/modos/ModoContragolpe.h"

ResultadoModo ModoContragolpe::paso(const PasoModo& p) {
    if (p.deFrente) {
        return ResultadoModo::Ataca;                          // lo ve de frente: a fondo
    }
    if (p.t >= ContragolpeTiempoMax) {
        return ResultadoModo::Termina;
    }
    if (p.visto == 0) {
        // Quieto un rato esperándolo; si no aparece, avanza despacio a buscarlo
        const int vel = p.t < ContragolpeTiempoQuieto ? 0 : ContragolpeVelocidadAvanceEspera;
        p.mando.mover(p.motor, vel, vel, true);
        return ResultadoModo::Sigue;
    }
    // Lo ve un sensor de 45° o lateral: se encara girando hacia él
    p.mando.mover(p.motor, p.visto * ContragolpeVelocidadGiro, -p.visto * ContragolpeVelocidadGiro, true);
    return ResultadoModo::Sigue;
}
