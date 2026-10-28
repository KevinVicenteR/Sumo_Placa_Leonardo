#include "movimiento/rumania/Modo1Defensivo.h"

ResultadoModo Modo1Defensivo::paso(const PasoModo& p) {
    // El frontal lo ve: 受け, lo recibe parado con la pala y después empuja a fondo
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
    if (p.visto == 0 && p.t < Modo1TiempoHenka) {
        p.mando.mover(p.motor, p.lado * Modo1VelocidadGiro, -p.lado * Modo1VelocidadGiro, true);
        return ResultadoModo::Sigue;
    }
    if (p.visto == 0) {
        // Quieto un rato esperándolo; si no aparece, avanza despacio a buscarlo
        const int vel = p.t < Modo1TiempoQuieto ? 0 : Modo1VelocidadAvanceEspera;
        p.mando.mover(p.motor, vel, vel, true);
        return ResultadoModo::Sigue;
    }
    // Lo ve un sensor de 45° o lateral: se encara girando hacia él
    p.mando.mover(p.motor, p.visto * Modo1VelocidadGiro, -p.visto * Modo1VelocidadGiro, true);
    return ResultadoModo::Sigue;
}
