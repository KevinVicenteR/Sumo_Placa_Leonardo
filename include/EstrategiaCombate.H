#ifndef ESTRATEGIA_COMBATE_H
#define ESTRATEGIA_COMBATE_H

#include "LecturasSensores.H"
#include "DecisionMovimiento.H"
#include "IEstrategiaCombate.H"

class EstrategiaCombate: public IEstrategiaCombate {
public:
    DecisionMovimiento decidir(const LecturasSensores& lecturas) const override;

private:
    DecisionMovimiento elegir(const LecturasSensores& lecturas) const;
    DecisionMovimiento elegirConPolitica(const LecturasSensores& lecturas) const;

    mutable int ladoUltimo = 0;  // 0 izquierda, 1 derecha: dónde se vio al enemigo por última vez
};

#endif
