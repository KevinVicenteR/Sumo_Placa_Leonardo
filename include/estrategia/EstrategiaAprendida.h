// =============================================================================
// EstrategiaAprendida.h — Estrategia aprendida por refuerzo en el simulador
// -----------------------------------------------------------------------------
// Convierte las lecturas en un número de estado y busca en TablaPolitica.h la
// acción que mejor funcionó en la simulación. Se activa con UsarPoliticaAprendida.
// =============================================================================
#ifndef ESTRATEGIA_APRENDIDA_H
#define ESTRATEGIA_APRENDIDA_H

#include "estrategia/EstrategiaCombate.h"

class EstrategiaAprendida: public EstrategiaCombate {
protected:
    DecisionMovimiento elegirContraEnemigo(const LecturasSensores& lecturas) override;

private:
    // Último lado donde se vio al rival: 0 = izquierda, 1 = derecha
    int ladoUltimo = 0;
};

#endif
