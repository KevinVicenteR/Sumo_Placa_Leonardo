#include "estrategia/EstrategiaPesos.h"
#include <Arduino.h>
#include "Parametros.h"

DecisionMovimiento EstrategiaPesos::elegirContraEnemigo(const LecturasSensores& lecturas) {
    const unsigned long ahora = millis();
    const bool algo = lecturas.latIzq || lecturas.c45Izq || lecturas.frontal || lecturas.c45Der ||
                      lecturas.latDer;
    if (algo) {
        ultimaVez = ahora;
    }
    if (!lecturas.frontal) {
        inicioFrontal = ahora;
    }
    int8_t x[redpesos::NumEntradas];
    redpesos::entradas(lecturas, ladoUltimo, x, !algo && ultimaVez != 0 && ahora - ultimaVez < RecuerdoRivalMs,
                       ahora - inicioFrontal >= ContactoFrontalMs);
    constexpr int minima = redpesos::sumaMinima(FuncionActivacion, UmbralActivacion);
    const int neurona = redpesos::elegirPorSuma(x, minima, sumas);
    // Recordar el lado donde se vio al rival por última vez
    if (lecturas.c45Izq || lecturas.latIzq) {
        ladoUltimo = 0;
    } else if (lecturas.c45Der || lecturas.latDer) {
        ladoUltimo = 1;
    }
    return {redpesos::Acciones[neurona], lecturas.frontal};
}
