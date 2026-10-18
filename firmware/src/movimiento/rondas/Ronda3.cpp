#include "movimiento/rondas/Ronda3.h"

ResultadoRonda Ronda3::paso(const PasoRonda& p) {
    if (p.deFrente) {
        return ResultadoRonda::Ataca;                         // lo ve de frente: a fondo
    }
    if (p.t >= Ronda3TiempoMax) {
        return ResultadoRonda::Termina;
    }
    if (p.visto == 0) {
        // Quieto un rato esperándolo; si no aparece, avanza despacio a buscarlo
        const int vel = p.t < Ronda3TiempoQuieto ? 0 : Ronda3VelocidadAvanceEspera;
        p.mando.mover(p.motor, vel, vel, true);
        return ResultadoRonda::Sigue;
    }
    // Lo ve un sensor de 45° o lateral: se encara girando hacia él
    p.mando.mover(p.motor, p.visto * Ronda3VelocidadGiro, -p.visto * Ronda3VelocidadGiro, true);
    return ResultadoRonda::Sigue;
}
