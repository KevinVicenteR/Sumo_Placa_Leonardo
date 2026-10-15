// RedLogica.h — Red de lógica: qué hacer en cada momento
// Perceptrón con una capa oculta (TablaRedLogica.h): 12 entradas 0/1 (los
// sensores de enemigo, el último lado visto, el piso cerca del borde, rival
// recién perdido, empujando, borde delante según el estimador, esquivando y
// rutina de inicio) -> 14 neuronas ReLU -> una suma por acción. Gana la mayor;
// si no llega a la suma mínima, el robot busca. Solo enteros.
#ifndef RED_LOGICA_H
#define RED_LOGICA_H

#include <stdint.h>
#include "estrategia/DecisionMovimiento.h"

namespace redlogica {

constexpr int NumEntradasRed = 12;
constexpr int NumAcciones = 10;

// Acción de cada salida (mismo orden que las filas de salida de la tabla)
constexpr TipoAccion Acciones[NumAcciones] = {
    TipoAccion::AtaqueFrontal, TipoAccion::AjusteIzq,   TipoAccion::AjusteDer,
    TipoAccion::CorregirIzq,   TipoAccion::CorregirDer, TipoAccion::DefensaIzq,
    TipoAccion::DefensaDer,    TipoAccion::Busqueda,    TipoAccion::EsquivarIzq,
    TipoAccion::EsquivarDer,
};
constexpr int SalidaBusqueda = 7;

// Salida ganadora (índice en Acciones). Rellena sumas[] si no es nulo.
int elegir(const int8_t x[NumEntradasRed], int minima, long* sumas = nullptr);

#if !defined(__AVR__)
// Pesos modificables (el simulador los carga con --red)
extern int8_t Parametros[];
int numParametros();
#endif

}

#endif
