#ifndef RED_LOGICA_H
#define RED_LOGICA_H

#include <stdint.h>
#include "estrategia/DecisionMovimiento.h"

namespace redlogica {
constexpr int NumEntradasRed = 12;
constexpr int NumAcciones = 10;
constexpr int NumOcultasRed = 14;
constexpr int NumSubredesRed = 3;

constexpr TipoAccion Acciones[NumAcciones] = {
    TipoAccion::AtaqueFrontal, TipoAccion::AjusteIzq,   TipoAccion::AjusteDer,
    TipoAccion::CorregirIzq,   TipoAccion::CorregirDer, TipoAccion::DefensaIzq,
    TipoAccion::DefensaDer,    TipoAccion::Busqueda,    TipoAccion::EsquivarIzq,
    TipoAccion::EsquivarDer,
};
constexpr int SalidaBusqueda = 7;

int elegir(const int8_t x[NumEntradasRed], int minima, long* sumas = nullptr);
int subredElegida();

#if !defined(__AVR__)
extern int8_t Parametros[];
int numParametros();
#endif
}

#endif
