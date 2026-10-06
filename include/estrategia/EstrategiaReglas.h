// EstrategiaReglas.h — Estrategia escrita a mano (la que usa el robot)
// -----------------------------------------------------------------------------
// Prioridades: rival de frente > rival a 45° > rival de lado > buscar.
#ifndef ESTRATEGIA_REGLAS_H
#define ESTRATEGIA_REGLAS_H

#include "estrategia/EstrategiaCombate.h"

class EstrategiaReglas: public EstrategiaCombate {
protected:
    DecisionMovimiento elegirContraEnemigo(const LecturasSensores& lecturas) override;
};

#endif
