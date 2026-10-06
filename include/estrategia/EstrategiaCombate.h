// EstrategiaCombate.h — Base común de todas las estrategias
// -----------------------------------------------------------------------------
// Patrón Método Plantilla (Template Method): decidir() fija los pasos que toda
// estrategia respeta y deja a cada subclase solo la parte que cambia:
//   1. Si hay línea blanca, evadir el borde (siempre tiene prioridad).
//   2. Si los tres sensores delanteros ven al rival, atacar de frente.
//   3. Si no, la subclase decide qué hacer con el rival (elegirContraEnemigo).
#ifndef ESTRATEGIA_COMBATE_H
#define ESTRATEGIA_COMBATE_H

#include "estrategia/IEstrategiaCombate.h"

class EstrategiaCombate: public IEstrategiaCombate {
public:
    DecisionMovimiento decidir(const LecturasSensores& lecturas) override;

protected:
    // Paso 3: lo que cambia entre estrategias
    virtual DecisionMovimiento elegirContraEnemigo(const LecturasSensores& lecturas) = 0;

private:
    DecisionMovimiento elegir(const LecturasSensores& lecturas);
};

#endif
