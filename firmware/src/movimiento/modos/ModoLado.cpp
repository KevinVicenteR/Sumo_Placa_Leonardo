#include "movimiento/modos/ModoLado.h"

ResultadoModo ModoLado::paso(const PasoModo& p) {
    if (p.deFrente) {
        return ResultadoModo::Ataca;                          // lo ve de frente: a fondo
    }
    if (p.t >= LadoTiempoMax) {
        return ResultadoModo::Termina;
    }
    // Lado del rival: el del sensor que lo ve o, si ninguno lo ve, el de DIP3
    if (p.visto != 0) {
        sentido = p.visto;
    } else if (sentido == 0) {
        sentido = p.lado;
    }
    // Pivota sobre la rueda de ese lado: esa rueda parada y la otra adelante
    const int v = p.t < LadoTiempoGiro || p.visto != 0 ? LadoVelocidadGiro : LadoVelocidadBarrido;
    p.mando.mover(p.motor, sentido > 0 ? v : 0, sentido > 0 ? 0 : v, true);
    return ResultadoModo::Sigue;
}
