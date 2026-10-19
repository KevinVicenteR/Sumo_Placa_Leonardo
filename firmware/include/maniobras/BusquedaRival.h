#ifndef BUSQUEDA_RIVAL_H
#define BUSQUEDA_RIVAL_H

#include "movimiento/MandoMotores.h"
#include "estrategia/DecisionMovimiento.h"

class BusquedaRival {
public:
    explicit BusquedaRival(int patronBusqueda = PatronBusqueda): patron(patronBusqueda) {}

    void ejecutar(MandoMotores& mando, IMotor& motor, unsigned long ahora);

    void detener() { buscando = false; }
    void cancelar() { buscando = false; evasionCompleta = false; }
    void terminoEvasion(bool sinRival) { evasionCompleta = sinRival; }

    int sentido() const { return sentidoGiro; }
    void fijarSentido(int sentidoNuevo) { sentidoGiro = sentidoNuevo; }
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
