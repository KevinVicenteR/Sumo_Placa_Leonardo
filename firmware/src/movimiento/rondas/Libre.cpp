#include "movimiento/rondas/Libre.h"

ResultadoRonda Libre::paso(const PasoRonda& p) {
    if (p.deFrente) {
        return ResultadoRonda::Ataca;                         // lo ve de frente: a fondo
    }
    if (p.t >= LibreTiempoMax) {
        return ResultadoRonda::Termina;
    }
    if (p.t < LibreTiempoArco) {
        // Arco a fondo abriéndose hacia DIP3
        p.mando.mover(p.motor, p.lado > 0 ? LibreVelocidadArco : LibreVelocidadArcoInterior,
                      p.lado > 0 ? LibreVelocidadArcoInterior : LibreVelocidadArco, true);
        return ResultadoRonda::Sigue;
    }
    // Después, giro hacia dentro o hacia el sensor de 45° o lateral que lo ve
    const int sentido = p.visto != 0 ? p.visto : -p.lado;
    p.mando.mover(p.motor, sentido * LibreVelocidadGiro, -sentido * LibreVelocidadGiro, true);
    return ResultadoRonda::Sigue;
}
