#ifndef ESTRATEGIA_COMBATE_H
#define ESTRATEGIA_COMBATE_H

#include "estrategia/IEstrategiaCombate.h"

class EstrategiaCombate: public IEstrategiaCombate {
public:
    DecisionMovimiento decidir(const LecturasSensores& lecturas) override;

protected:
    virtual DecisionMovimiento elegirContraEnemigo(const LecturasSensores& lecturas) = 0;

private:
    DecisionMovimiento elegir(const LecturasSensores& lecturas);
};

#endif
