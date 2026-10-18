#include "movimiento/modos/ModoArco.h"

ResultadoModo ModoArco::paso(const PasoModo& p) {
    if (p.deFrente) {
        return ResultadoModo::Ataca;                          // lo ve de frente: a fondo
    }
    if (p.t >= ArcoTiempoMax) {
        return ResultadoModo::Termina;
    }
    if (p.t < ArcoTiempo) {
        // Arco a fondo abriéndose hacia DIP3
        p.mando.mover(p.motor, p.lado > 0 ? ArcoVelocidad : ArcoVelocidadInterior,
                      p.lado > 0 ? ArcoVelocidadInterior : ArcoVelocidad, true);
        return ResultadoModo::Sigue;
    }
    // Después, giro hacia dentro o hacia el sensor de 45° o lateral que lo ve
    const int sentido = p.visto != 0 ? p.visto : -p.lado;
    p.mando.mover(p.motor, sentido * ArcoVelocidadGiro, -sentido * ArcoVelocidadGiro, true);
    return ResultadoModo::Sigue;
}
