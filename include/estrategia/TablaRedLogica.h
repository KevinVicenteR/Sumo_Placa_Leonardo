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
      22,   -5,   10,   -1,   12,   -9,    2,   -2,    3,    4,   -4,    5,    -5,
      -3,   21,    5,   -3,   -5,   -7,    2,    7,    5,    3,    1,    2,    -3,
       4,    0,   20,   -6,   -5,    4,   -5,    2,    2,   -4,    5,   -4,     0,
       4,    3,   -2,   20,    5,   -1,   -3,    3,   -4,   -5,    7,    2,    13,
       8,    2,    6,   -5,   21,    4,    2,   -5,   -5,   -1,    0,   -1,     4,
     -10,   -9,    1,    2,   -3,   16,   -2,   -3,    5,   -1,    4,   -9,    -1,
       2,   11,    4,   -1,   -4,   -1,   16,    4,    0,    4,    3,    0,    -9,
      -1,   -6,    0,   -2,    3,    1,   -6,   20,    5,   -4,   -2,   -6,     3,
      -7,   -1,   -2,   -7,    1,   -9,    1,    4,   12,    3,    1,   -6,    -3,
      -7,   13,   -3,   -2,   -5,   -5,  -12,    0,    7,   21,    3,   -2,     3,
      -1,   -6,    1,  -10,   12,  -10,    7,   -3,   -6,    1,   12,    0,    -3,
      -6,   -4,   -2,    2,    6,    3,    0,   -5,    1,    0,   -6,   22,     8,
     -12,    0,   -8,   -3,    2,    2,    0,   -5,    1,    4,   -4,    2,     2,
       0,   -2,   -1,   -1,   -4,   -4,   -3,   -4,    4,   -2,    2,   -1,    -4,
    // salidas
      43,    3,    3,   -3,    5,   -4,    6,    0,    2,    5,    3,    6,  -19,    4,     1,   // AtaqueFrontal
      32,    7,  -15,    9,    3,   -2,  -14,    2,   -3,   11,   -3,   -2,  -24,    2,    -1,   // AjusteIzq
      35,   -9,   13,   -5,    6,   -8,    8,   -2,   -5,   -9,   -4,    3,  -27,   -1,    -5,   // AjusteDer
     -46,   25,    2,    0,   -3,    1,   -4,    3,   -1,    5,    0,   -1,   -4,    5,    -1,   // CorregirIzq
     -49,   -2,   41,   -1,   -9,   -8,    1,    9,    2,   -5,    3,   -7,  -10,    0,     3,   // CorregirDer
     -45,  -43,  -35,   21,   -2,   -3,    0,   -4,    4,   -3,   -4,    4,   -4,    0,     6,   // DefensaIzq
     -29,  -42,  -38,    4,   24,    5,    0,    2,   -1,   -8,   -7,    5,   -7,   -3,     3,   // DefensaDer
     -36,  -39,  -40,  -41,  -50,    1,    0,    4,    0,    6,    1,   10,    7,    6,    -7,   // Busqueda
       1,   -5,   -5,    1,   -2,    0,    5,    1,    4,   -3,   -2,   -6,  -98,    5,     5,   // EsquivarIzq
      -6,   -6,   -4,    1,    0,    1,    2,    2,   -3,   -4,    1,   -1, -105,    6,     3,   // EsquivarDer
};

}

#endif
