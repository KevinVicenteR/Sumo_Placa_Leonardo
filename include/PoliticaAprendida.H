#ifndef POLITICA_APRENDIDA_H
#define POLITICA_APRENDIDA_H

#include <stdint.h>
#include "DecisionMovimiento.H"
#include "LecturasSensores.H"

namespace politica {

constexpr int NumEstados = 64;
constexpr int NumAcciones = 8;

constexpr TipoAccion Acciones[NumAcciones] = {
    TipoAccion::AtaqueFrontal, TipoAccion::AjusteIzq,  TipoAccion::AjusteDer,
    TipoAccion::CorregirIzq,   TipoAccion::CorregirDer, TipoAccion::DefensaIzq,
    TipoAccion::DefensaDer,    TipoAccion::Busqueda,
};

int codificarEstado(const LecturasSensores& lecturas, int ladoUltimo);

int accionManual(int estado);

int elegirAccion(int estado);

}

#endif
