#include "movimiento/rondas/Ronda1.h"

ResultadoRonda Ronda1::paso(const PasoRonda& p) {
    // Frenando el giro tras encontrar al rival
    if (frenando) {
        if (p.ahora - inicioFreno < freno.duracion) {
            p.mando.mover(p.motor, freno.izq, freno.der, true, true);   // freno: sin pivote
            return ResultadoRonda::Sigue;
        }
        return ResultadoRonda::Termina;
    }
    // Primero se aparta del rival avanzando recto
    if (p.t < Ronda1TiempoEsquiva) {
        p.mando.mover(p.motor, Ronda1VelocidadEsquiva, Ronda1VelocidadEsquiva, true);
        return ResultadoRonda::Sigue;
    }
    // El giro cuenta su tiempo desde que termina la esquiva. Una detección no lo
    // termina antes de su mínimo ni sin confirmarse
    const unsigned long t = p.t - Ronda1TiempoEsquiva;
    const bool confirmado = rivalVisto.actualizar(veDeFrente(p.decision), p.ahora);
    if (confirmado && t >= Ronda1TiempoMinimoGiro) {
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
    if (t >= Ronda1TiempoMaxGiro) {
        return ResultadoRonda::Termina;
    }
    // Media vuelta en el sitio hacia el lado de DIP3
    p.mando.mover(p.motor, p.lado * Ronda1VelocidadGiro, -p.lado * Ronda1VelocidadGiro, true);
    return ResultadoRonda::Sigue;
}
