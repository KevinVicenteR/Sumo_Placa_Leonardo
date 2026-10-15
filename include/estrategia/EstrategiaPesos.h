// EstrategiaPesos.h — Estrategia por pesos, sesgos y función de activación
// Pasa las lecturas por la red de RedPesos.h y hace la acción de la neurona
// ganadora. Se activa con UsarRedPesos (Parametros.h). El borde y el ataque
// directo siguen teniendo prioridad (ver EstrategiaCombate).
#ifndef ESTRATEGIA_PESOS_H
#define ESTRATEGIA_PESOS_H

#include "estrategia/EstrategiaCombate.h"
#include "estrategia/RedPesos.h"
#include "Parametros.h"

class EstrategiaPesos: public EstrategiaCombate {
public:
    // Activación de cada neurona en el último ciclo (telemetría y tests)
    float activacion(int neurona) const { return redpesos::activar(sumas[neurona], FuncionActivacion); }

protected:
    DecisionMovimiento elegirContraEnemigo(const LecturasSensores& lecturas) override;

private:
    // Último lado donde se vio al rival: 0 = izquierda, 1 = derecha
    int ladoUltimo = 0;
    // Para las entradas de tiempo: última vez que vio al rival y desde cuándo
    // lo ve sin parar el frontal
    unsigned long ultimaVez = 0;
    unsigned long inicioFrontal = 0;
    // Suma ponderada de cada neurona en el último ciclo
    int16_t sumas[redpesos::NumNeuronas] = {};
};

#endif
