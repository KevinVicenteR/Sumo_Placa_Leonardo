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
