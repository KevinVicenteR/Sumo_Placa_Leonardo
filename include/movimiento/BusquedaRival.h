// =============================================================================
// BusquedaRival.h — Movimiento cuando no se ve al rival
// -----------------------------------------------------------------------------
// Dos patrones (PatronBusqueda):
//   1 = arcos suaves que cambian de lado cada TiempoArcoBusqueda.
//   0 = avanzar, parar, girar en el sitio, parar... (en ciclo).
// Recuerda el último lado donde se vio al rival para girar hacia ahí.
// =============================================================================
#ifndef BUSQUEDA_RIVAL_H
#define BUSQUEDA_RIVAL_H

#include "movimiento/MandoMotores.h"
#include "estrategia/DecisionMovimiento.h"

class BusquedaRival {
public:
    explicit BusquedaRival(int patronBusqueda = PatronBusqueda): patron(patronBusqueda) {}

    // Un ciclo de búsqueda
    void ejecutar(MandoMotores& mando, IMotor& motor, unsigned long ahora);

    // La próxima búsqueda empezará desde el principio
    void detener() { buscando = false; }
    // Además olvida que acaba de terminar una evasión
    void cancelar() { buscando = false; evasionCompleta = false; }
    // Al terminar una evasión: si no hay rival a la vista, el patrón 0 empieza avanzando
    void terminoEvasion(bool sinRival) { evasionCompleta = sinRival; }

    // Lado hacia el que girar: 1 = derecha, -1 = izquierda
    int sentido() const { return sentidoGiro; }
    void fijarSentido(int sentidoNuevo) { sentidoGiro = sentidoNuevo; }
    // Si la acción indica dónde está el rival, gira hacia ese lado al buscar
    void recordarLadoEnemigo(TipoAccion tipo);

private:
    void buscarEnArcos(MandoMotores& mando, IMotor& motor, unsigned long ahora);
    void avanzarPorPulsos(MandoMotores& mando, IMotor& motor, unsigned long t);

    int patron;
    int sentidoGiro = 1;
    bool buscando = false;
    bool evasionCompleta = false;
    unsigned long inicioBusqueda = 0;
};

#endif
