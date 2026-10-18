#include "movimiento/rumania/Modo3Predictivo.h"

ResultadoModo Modo3Predictivo::paso(const PasoModo& p) {
    if (p.deFrente) {
        return ResultadoModo::Ataca;                          // lo ve de frente: a fondo
    }
    if (p.t >= Modo3TiempoMax) {
        return ResultadoModo::Termina;
    }
    if (p.visto != 0) {
        // Intercepta: avanza en arco hacia el lado en que lo ve
        ultimoVisto = p.visto;
        const int ext = Modo3VelocidadArco, inte = Modo3VelocidadArcoInterior;
        p.mando.mover(p.motor, p.visto > 0 ? ext : inte, p.visto > 0 ? inte : ext, true);
        return ResultadoModo::Sigue;
    }
    // Sin verlo: barre hacia donde lo vio por última vez (al empezar, hacia DIP3)
    const int sentido = ultimoVisto != 0 ? ultimoVisto : p.lado;
    p.mando.mover(p.motor, sentido * Modo3VelocidadBarrido, -sentido * Modo3VelocidadBarrido, true);
    return ResultadoModo::Sigue;
}
