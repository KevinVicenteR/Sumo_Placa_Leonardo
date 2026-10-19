#ifndef ESTRATEGIA_REGLAS_H
#define ESTRATEGIA_REGLAS_H

#include "estrategia/EstrategiaCombate.h"

class EstrategiaReglas: public EstrategiaCombate {
protected:
    DecisionMovimiento elegirContraEnemigo(const LecturasSensores& lecturas) override;
};

#endif
