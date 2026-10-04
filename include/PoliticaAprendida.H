#ifndef POLITICA_APRENDIDA_H
#define POLITICA_APRENDIDA_H

#include <stdint.h>
#include "DecisionMovimiento.H"
#include "LecturasSensores.H"

// Política de combate aprendida por refuerzo en el simulador
// (simulacion/entrenar_politica.py). El robot no aprende en el combate: solo
// consulta la tabla resultante (TablaPolitica.H).
//
// Estado: qué sensores de enemigo lo ven (frontal, 45° izq/der, lateral izq/der)
// y hacia qué lado se vio por última vez: 2^5 × 2 = 64 estados.
// Acción: una de las maniobras de combate de ControlMovimiento.
// La evasión del borde no se aprende: tiene prioridad y es fija.
namespace politica {

constexpr int NumEstados = 64;
constexpr int NumAcciones = 8;

// Acciones en el orden de la tabla
constexpr TipoAccion Acciones[NumAcciones] = {
    TipoAccion::AtaqueFrontal, TipoAccion::AjusteIzq,  TipoAccion::AjusteDer,
    TipoAccion::CorregirIzq,   TipoAccion::CorregirDer, TipoAccion::DefensaIzq,
    TipoAccion::DefensaDer,    TipoAccion::Busqueda,
};

// ladoUltimo: 0 izquierda, 1 derecha
int codificarEstado(const LecturasSensores& lecturas, int ladoUltimo);

// La estrategia escrita a mano expresada como acción para cada estado
int accionManual(int estado);

// Acción para un estado según la tabla (al entrenar en el simulador, el
// simulador decide para explorar)
int elegirAccion(int estado);

}  // namespace politica

#endif
