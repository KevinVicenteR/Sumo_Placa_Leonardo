// IEstrategiaCombate.h — Interfaz de una estrategia de combate
// -----------------------------------------------------------------------------
// Patrón Estrategia (Strategy): hay varias formas de decidir (reglas escritas
// a mano o política aprendida) y el robot puede usar cualquiera sin cambiar el
// resto del código.
#ifndef IESTRATEGIA_COMBATE_H
#define IESTRATEGIA_COMBATE_H

#include "sensores/LecturasSensores.h"
#include "estrategia/DecisionMovimiento.h"

class IEstrategiaCombate {
public:
    virtual ~IEstrategiaCombate() {}
    virtual DecisionMovimiento decidir(const LecturasSensores& lecturas) = 0;
};

#endif
