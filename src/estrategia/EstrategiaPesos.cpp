#include "estrategia/EstrategiaPesos.h"
#include "Parametros.h"

DecisionMovimiento EstrategiaPesos::elegirContraEnemigo(const LecturasSensores& lecturas) {
    int8_t x[redpesos::NumEntradas];
    redpesos::entradas(lecturas, ladoUltimo, x);
    const int neurona = redpesos::elegirNeurona(x, FuncionActivacion, UmbralActivacion, activaciones);
    // Recordar el lado donde se vio al rival por última vez
    if (lecturas.c45Izq || lecturas.latIzq) {
        ladoUltimo = 0;
    } else if (lecturas.c45Der || lecturas.latDer) {
        ladoUltimo = 1;
    }
    return {redpesos::Acciones[neurona], lecturas.frontal};
}
