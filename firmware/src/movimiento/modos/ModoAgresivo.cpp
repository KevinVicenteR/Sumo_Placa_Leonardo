#include "movimiento/modos/ModoAgresivo.h"

ResultadoModo ModoAgresivo::paso(const PasoModo& p) {
    if (p.deFrente) {
        return ResultadoModo::Ataca;                          // lo ve de frente: a fondo
    }
    if (p.t >= AgresivoTiempoMax) {
        return ResultadoModo::Termina;
    }
    if (p.visto == 0) {
        p.mando.mover(p.motor, AgresivoVelocidadAvance, AgresivoVelocidadAvance, true);   // recto
    } else {
        // Lo ve un sensor de 45° o lateral: giro a fondo hacia él
        p.mando.mover(p.motor, p.visto * AgresivoVelocidadGiro, -p.visto * AgresivoVelocidadGiro, true);
    }
    return ResultadoModo::Sigue;
}
