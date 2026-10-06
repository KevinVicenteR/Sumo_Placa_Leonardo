// PoliticaAprendida.h — Tabla estado -> acción de la estrategia aprendida
// -----------------------------------------------------------------------------
// Estado = 6 bits: cinco sensores de enemigo + último lado visto (64 estados).
// Acción = índice en Acciones (8 acciones posibles).
#ifndef POLITICA_APRENDIDA_H
#define POLITICA_APRENDIDA_H

#include <stdint.h>
#include "estrategia/DecisionMovimiento.h"
#include "sensores/LecturasSensores.h"

namespace politica {

constexpr int NumEstados = 64;
constexpr int NumAcciones = 8;

constexpr TipoAccion Acciones[NumAcciones] = {
    TipoAccion::AtaqueFrontal, TipoAccion::AjusteIzq,  TipoAccion::AjusteDer,
    TipoAccion::CorregirIzq,   TipoAccion::CorregirDer, TipoAccion::DefensaIzq,
    TipoAccion::DefensaDer,    TipoAccion::Busqueda,
};

// Lecturas + último lado -> número de estado (0-63)
int codificarEstado(const LecturasSensores& lecturas, int ladoUltimo);

// Acción que elegiría EstrategiaReglas en ese estado (punto de partida del entrenamiento)
int accionManual(int estado);

// Acción de la tabla aprendida para ese estado
int elegirAccion(int estado);

}

#endif
