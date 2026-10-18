#include "movimiento/modos/ModoDefensivo.h"

ResultadoModo ModoDefensivo::paso(const PasoModo& p) {
    // El frontal lo ve: 受け, lo recibe parado con la pala y después empuja a fondo
    if (!p.deFrente) {
        recibiendo = false;
    } else {
        if (!recibiendo) {
            recibiendo = true;
            inicioRecibir = p.ahora;
        }
        if (p.ahora - inicioRecibir < DefensivoTiempoRecibir) {
            p.mando.mover(p.motor, 0, 0, true);
            return ResultadoModo::Sigue;
        }
        return ResultadoModo::Ataca;
    }
    if (p.t >= DefensivoTiempoMax) {
        return ResultadoModo::Termina;
    }
    if (p.visto == 0) {
        // Quieto un rato esperándolo; si no aparece, avanza despacio a buscarlo
        const int vel = p.t < DefensivoTiempoQuieto ? 0 : DefensivoVelocidadAvanceEspera;
        p.mando.mover(p.motor, vel, vel, true);
        return ResultadoModo::Sigue;
    }
    // Lo ve un sensor de 45° o lateral: se encara girando hacia él
    p.mando.mover(p.motor, p.visto * DefensivoVelocidadGiro, -p.visto * DefensivoVelocidadGiro, true);
    return ResultadoModo::Sigue;
}
