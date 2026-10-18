#include "movimiento/rondas/Ronda2.h"

ResultadoRonda Ronda2::paso(const PasoRonda& p) {
    if (!iniciada) {
        iniciada = true;
        lado = p.lado;
    }
    // Frenando el giro tras encontrar al rival
    if (frenando) {
        if (p.ahora - inicioFreno < freno.duracion) {
            p.mando.mover(p.motor, freno.izq, freno.der, true, true);   // freno: sin pivote
            return ResultadoRonda::Sigue;
        }
        return ResultadoRonda::Termina;
    }
    // El lado del giro lo decide el primer sensor lateral o de 45° que lo vea,
    // también durante la esquiva (al avanzar, el rival queda atrás y el sensor
    // lateral deja de verlo)
    decidirLado(p);
    if (p.t < Ronda2TiempoEsquiva) {
        p.mando.mover(p.motor, Ronda2VelocidadEsquiva, Ronda2VelocidadEsquiva, true);
        return ResultadoRonda::Sigue;
    }
    const unsigned long t = p.t - Ronda2TiempoEsquiva;
    const bool confirmado = rivalVisto.actualizar(veDeFrente(p.decision), p.ahora);
    if (confirmado && t >= Ronda2TiempoMinimoGiro) {
        // Rival encontrado: embestida, frenando antes el giro si va rápido
        p.ataque.iniciarEmbestida(p.ahora);
        if (p.mando.calcularFreno(TiempoFrenadoRuedas, freno)) {
            frenando = true;
            inicioFreno = p.ahora;
            p.mando.mover(p.motor, freno.izq, freno.der, true, true);   // freno: sin pivote
            return ResultadoRonda::Sigue;
        }
        return ResultadoRonda::Termina;
    }
    if (t >= Ronda2TiempoMaxGiro) {
        return ResultadoRonda::Termina;
    }
    if (Ronda2Pivote) {
        // Pivote sobre la rueda del lado del rival
        p.mando.mover(p.motor, lado > 0 ? Ronda2VelocidadPivote : 0, lado > 0 ? 0 : Ronda2VelocidadPivote, true);
    } else {
        p.mando.mover(p.motor, lado * Ronda2VelocidadGiro, -lado * Ronda2VelocidadGiro, true);
    }
    return ResultadoRonda::Sigue;
}

// Un sensor lateral o de 45° que ve al rival fija el lado del giro (si ninguno
// lo ve, se usa el lado de DIP3). Una vez decidido, no cambia.
void Ronda2::decidirLado(const PasoRonda& p) {
    if (ladoDecidido) {
        return;
    }
    const TipoAccion t = p.decision.tipo;
    const bool izquierda = t == TipoAccion::DefensaIzq || t == TipoAccion::CorregirIzq;
    const bool derecha = t == TipoAccion::DefensaDer || t == TipoAccion::CorregirDer;
    if (!izquierda && !derecha) {
        return;
    }
    lado = izquierda ? -1 : 1;
    p.busqueda.fijarSentido(lado);
    ladoDecidido = true;
}
