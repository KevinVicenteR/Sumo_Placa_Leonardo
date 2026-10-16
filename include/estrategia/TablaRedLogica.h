// TablaRedLogica.h — Pesos de la red de lógica (los escribe simulacion/entrenar_redes.py)
// 12 entradas -> 14 neuronas ReLU -> 10 salidas, enteros int8.
// Cada fila: los pesos de una neurona y, al final, su sesgo. Primero las
// 14 neuronas ocultas (entradas: Fr, 45I, 45D, LatI, LatD, Lado, Borde, Perd, Emp, BordeDelante, Esquivando, Rutina) y después las salidas.
#ifndef TABLA_RED_LOGICA_H
#define TABLA_RED_LOGICA_H

#include "estrategia/TablaRed.h"

namespace redlogica {

constexpr int NumEntradas = 12;
constexpr int NumOcultas = 14;
constexpr int NumSalidas = 10;
constexpr int NumParametros = NumOcultas * (NumEntradas + 1) + NumSalidas * (NumOcultas + 1);

TABLA_RED(Parametros, NumParametros) = {
      21,   -5,    8,    0,   11,   -5,    1,   -2,    2,    4,   -3,    6,    -2,
      -2,   21,    4,    2,   -6,   -7,    2,    6,    7,    5,    0,    2,    -1,
       4,    0,   18,   -4,   -6,    2,   -4,    1,    2,   -5,    3,   -2,    -1,
       3,    3,   -4,   16,    3,   -3,   -3,    2,   -2,   -4,    8,    2,    10,
       7,    2,    8,   -4,   20,    3,    3,   -3,   -3,    1,   -1,    2,     4,
      -9,   -9,    1,    3,    1,   15,   -4,   -2,    5,    1,    3,   -6,    -1,
       1,    9,    4,   -1,   -6,   -2,   15,    2,   -1,    3,    2,    0,    -9,
      -1,   -7,   -1,   -2,    3,    0,   -5,   19,    4,   -3,    0,   -5,     2,
      -6,    0,    0,   -7,    1,  -10,    0,    4,   11,    0,    1,   -5,    -3,
      -4,   10,   -3,   -3,   -5,   -1,   -8,    1,    4,   19,    1,   -3,     2,
      -1,   -4,   -1,  -10,   10,   -9,    6,    0,   -6,    2,   12,    2,     1,
      -7,   -5,    0,    3,    3,    0,    1,   -5,    1,   -1,   -5,   22,     9,
     -10,   -1,   -7,   -2,    0,    0,    1,   -6,    3,    3,   -4,    3,     2,
       1,   -2,    0,    3,   -5,   -1,   -3,   -5,    3,   -2,    1,   -1,    -4,
    // salidas
      43,    3,    1,   -1,    4,   -2,    5,   -2,    2,    4,    4,    7,  -17,    4,     2,   // AtaqueFrontal
      33,    8,  -14,    7,    4,   -1,  -10,   -1,   -2,    9,   -3,   -1,  -23,    2,    -4,   // AjusteIzq
      36,   -8,   12,   -5,    4,   -7,    7,   -1,   -5,   -8,   -2,    1,  -26,   -1,    -5,   // AjusteDer
     -45,   26,    3,    3,   -3,    1,   -5,    3,    0,    4,   -2,   -3,   -5,    5,    -3,   // CorregirIzq
     -49,   -1,   40,   -3,   -6,   -5,   -1,   11,    2,   -4,    2,   -6,  -12,    0,     2,   // CorregirDer
     -46,  -42,  -34,   19,   -2,   -3,    0,   -2,    4,   -1,   -2,    1,   -4,    1,     3,   // DefensaIzq
     -33,  -43,  -39,    2,   24,    3,   -3,    0,   -2,   -7,   -8,    3,   -7,   -4,     3,   // DefensaDer
     -35,  -39,  -41,  -40,  -49,   -1,    0,    4,   -2,    5,    1,    9,    7,    6,    -6,   // Busqueda
       1,   -3,   -2,    2,    0,    2,    5,   -1,    4,   -4,   -2,   -6,  -98,    2,     4,   // EsquivarIzq
      -5,   -6,   -4,    0,    0,    1,    1,    5,   -4,   -5,    2,   -2, -105,    3,     2,   // EsquivarDer
};

}

#endif
