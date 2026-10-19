#include "modos_rumania/Modo3Lado.h"

ResultadoModo Modo3Lado::paso(const PasoModo& p) {
    if (p.deFrente) {
        return ResultadoModo::Ataca;
    }
    if (p.t >= Modo3TiempoMax) {
        return ResultadoModo::Termina;
    }
    if (p.visto != 0) {
        sentido = p.visto;
    } else if (sentido == 0) {
        sentido = p.lado;
    }
    const int v = p.t < Modo3TiempoGiro || p.visto != 0 ? Modo3VelocidadGiro : Modo3VelocidadBarrido;
    p.mando.mover(p.motor, sentido > 0 ? v : 0, sentido > 0 ? 0 : v, true);
    return ResultadoModo::Sigue;
}
