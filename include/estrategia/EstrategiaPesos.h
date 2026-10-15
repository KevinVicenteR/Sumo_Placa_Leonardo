// EstrategiaPesos.h — Estrategia por pesos, sesgos y función de activación
// Pasa las lecturas por la red de RedPesos.h y hace la acción de la neurona
// ganadora. Se activa con UsarRedPesos (Parametros.h). El borde y el ataque
// directo siguen teniendo prioridad (ver EstrategiaCombate).
#ifndef ESTRATEGIA_PESOS_H
#define ESTRATEGIA_PESOS_H

#include "estrategia/EstrategiaCombate.h"
#include "estrategia/RedPesos.h"

class EstrategiaPesos: public EstrategiaCombate {
public:
    // Activación de cada neurona en el último ciclo (telemetría y tests)
    float activacion(int neurona) const { return activaciones[neurona]; }

protected:
    DecisionMovimiento elegirContraEnemigo(const LecturasSensores& lecturas) override;

private:
    // Último lado donde se vio al rival: 0 = izquierda, 1 = derecha
    int ladoUltimo = 0;
    float activaciones[redpesos::NumNeuronas] = {};
};

#endif
