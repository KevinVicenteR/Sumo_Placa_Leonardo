#include "estrategia/EstrategiaAprendida.h"
#include "estrategia/PoliticaAprendida.h"

DecisionMovimiento EstrategiaAprendida::elegirContraEnemigo(const LecturasSensores& lecturas) {
    const int estado = politica::codificarEstado(lecturas, ladoUltimo);
    // Recordar el lado donde se vio al rival por última vez
    if (lecturas.c45Izq || lecturas.latIzq) {
        ladoUltimo = 0;
    } else if (lecturas.c45Der || lecturas.latDer) {
        ladoUltimo = 1;
    }
    return {politica::Acciones[politica::elegirAccion(estado)], lecturas.frontal};
}
