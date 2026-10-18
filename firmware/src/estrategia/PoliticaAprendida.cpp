#include "estrategia/PoliticaAprendida.h"
#include "estrategia/TablaPolitica.h"

namespace politica {

namespace {
// Bit de cada dato dentro del número de estado
const int Frontal = 1, C45Izq = 2, C45Der = 4, LatIzq = 8, LatDer = 16, LadoDer = 32;

// Posición de una acción dentro de Acciones
int indice(TipoAccion tipo) {
    for (int i = 0; i < NumAcciones; i++) {
        if (Acciones[i] == tipo) return i;
    }
    return NumAcciones - 1;
}
}

int codificarEstado(const LecturasSensores& l, int ladoUltimo) {
    return (l.frontal ? Frontal : 0) | (l.c45Izq ? C45Izq : 0) | (l.c45Der ? C45Der : 0) |
           (l.latIzq ? LatIzq : 0) | (l.latDer ? LatDer : 0) | (ladoUltimo ? LadoDer : 0);
}

int accionManual(int e) {
    if (e & Frontal) {
        if ((e & C45Izq) && !(e & C45Der)) return indice(TipoAccion::AjusteIzq);
        if ((e & C45Der) && !(e & C45Izq)) return indice(TipoAccion::AjusteDer);
        return indice(TipoAccion::AtaqueFrontal);
    }
    if (e & C45Izq) return indice(TipoAccion::CorregirIzq);
    if (e & C45Der) return indice(TipoAccion::CorregirDer);
    if (e & LatIzq) return indice(TipoAccion::DefensaIzq);
    if (e & LatDer) return indice(TipoAccion::DefensaDer);
    return indice(TipoAccion::Busqueda);
}

#ifdef ENTRENAMIENTO_POLITICA
// Durante el entrenamiento (simulador) la acción la elige el entrenador
int elegirAccionEntrenamiento(int estado, int accionTabla);

int elegirAccion(int estado) {
    return elegirAccionEntrenamiento(estado, TablaPolitica[estado]);
}
#else
int elegirAccion(int estado) {
    return TablaPolitica[estado];
}
#endif

}
