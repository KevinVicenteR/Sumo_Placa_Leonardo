// TablaRedLogica.h — Pesos de la red de lógica (los escribe simulacion/entrenar_redes.py)
// Red principal + subredes, enteros int8. Red principal: 12 entradas -> 14
// neuronas ReLU (entradas: Fr, 45I, 45D, LatI, LatD, Lado, Borde, Perd, Emp, BordeDelante, Esquivando, Rutina) y un selector que elige
// una de las 3 subredes; la subred elegida da una suma por acción y gana la
// mayor. Cada fila: los pesos de una neurona y, al final, su sesgo. Orden: capa
// oculta, subred 0, selector, subredes 1..2.
#ifndef TABLA_RED_LOGICA_H
#define TABLA_RED_LOGICA_H

#include "estrategia/TablaRed.h"

namespace redlogica {

constexpr int NumEntradas = 12;
constexpr int NumOcultas = 14;
constexpr int NumSalidas = 10;
constexpr int NumSubredes = 3;
constexpr int NumParametros = NumOcultas * (NumEntradas + 1) + NumSubredes * NumSalidas * (NumOcultas + 1) +
                              NumSubredes * (NumOcultas + 1);

TABLA_RED(Parametros, NumParametros) = {
      37,   -5,   -1,   11,   15,   -2,  -18,   -4,    7,    0,   -2,    9,   -15,
      -9,   32,    2,   12,    3,   -7,    8,  -13,    8,    2,   12,   -4,    -5,
      11,    3,   26,   -2,    3,   16,  -12,   13,    8,   -4,    9,  -14,    -6,
      14,  -12,   -6,   14,   -4,   -7,   -1,   17,   -9,   -1,   17,    0,    18,
       8,   -5,   11,   -6,   26,   13,    3,  -10,   12,    1,   -6,  -11,    17,
     -21,  -16,   -9,   -3,    8,   22,   -9,   -6,   -6,    9,   12,    5,   -18,
       9,    9,   15,  -13,   -9,  -21,   16,   18,    3,    4,   -1,    2,   -16,
       2,  -13,   -3,  -13,   -5,    8,    5,   20,    6,    3,    1,   -2,    11,
      -5,  -13,   -2,    3,   -1,    1,   -4,    3,   11,  -11,   -8,   -5,    -2,
       8,  -14,   -8,    3,   -9,  -11,  -15,    8,   22,    9,    8,   -3,   -14,
       0,    4,    0,    2,   -3,   -8,   17,    3,    1,   -1,   13,   -8,     8,
      -2,  -12,   12,    1,    2,  -21,    9,    4,    1,    1,  -15,   13,    20,
      -4,   -6,   -5,   -1,    7,    4,    6,   -7,  -12,   13,  -14,   -2,    10,
       6,    2,  -21,   25,    2,   -1,   -5,  -20,    7,   -1,   -5,   -7,     2,
    // subred 0 (una fila por acción)
      30,   -4,   -1,  -11,   -4,   -2,   15,   -7,   -1,   -3,    1,   -3,   -8,   10,     7,   // AtaqueFrontal
      45,    7,  -12,   14,    9,   10,    0,   -9,   -4,   11,    0,   -9,  -18,    5,   -14,   // AjusteIzq
      37,   -8,   12,   -4,    1,  -10,   11,   18,   -5,   -6,  -10,   15,  -16,    9,   -22,   // AjusteDer
     -44,   27,   -2,  -10,  -18,   -8,   -2,    3,   -3,    0,    0,  -10,   -1,    6,    -7,   // CorregirIzq
     -44,    7,   37,    0,   -4,   -1,   -4,   16,  -13,    2,   -5,  -10,  -17,   -2,     0,   // CorregirDer
     -44,  -37,  -30,   30,    5,   -9,   -6,   12,   15,    7,    9,   21,   -3,   -8,    15,   // DefensaIzq
     -34,  -41,  -41,   -1,   23,   -2,  -12,    2,   26,  -12,    5,    7,   -1,    0,    -4,   // DefensaDer
     -43,  -42,  -46,  -20,  -41,    3,    1,   -5,  -14,   -5,   -3,    6,    3,   13,   -11,   // Busqueda
      -2,   -2,   -1,   -4,   10,   -3,   11,  -10,   13,   -5,    0,   -5,  -97,    2,    12,   // EsquivarIzq
     -14,  -10,  -10,   -5,   13,    9,    5,    0,   -9,  -12,    0,    8, -104,  -11,   -14,   // EsquivarDer
    // selector: una fila por subred (gana la mayor)
      -4,   -4,   -5,   -1,   -1,    1,   -2,   -3,   -8,    1,   -7,   -3,    4,   -5,    19,   // subred 0
       2,    8,    0,    0,   -1,    2,   -2,    4,   -8,    1,   -6,    2,    0,    6,    -9,   // subred 1
      10,    5,    9,   -1,    0,    2,   -1,    5,    1,    2,    4,   -4,   -2,    7,     2,   // subred 2
    // subred 1
      47,    6,   -6,   -7,  -13,   -2,    1,    0,   -8,    3,    5,    3,  -18,    5,    13,   // AtaqueFrontal
      43,    5,  -23,   21,  -13,  -10,  -12,    5,   -6,   10,    2,    5,  -13,    0,    -3,   // AjusteIzq
      27,  -15,   15,    8,   11,  -19,    6,   -4,   -5,    5,   -1,    8,  -26,    0,    -1,   // AjusteDer
     -35,   31,   -3,    9,   -1,   -7,    2,    7,    4,   11,   11,   -7,  -15,    6,    -6,   // CorregirIzq
     -51,    6,   41,    2,  -17,   -5,    7,   -1,    7,  -11,    9,   -2,  -11,  -11,     4,   // CorregirDer
     -56,  -52,  -20,   28,    2,  -10,   -3,    4,    1,   16,    6,    7,  -19,    7,    13,   // DefensaIzq
     -30,  -42,  -42,    4,   23,   -4,   12,    1,   16,  -13,  -13,    6,  -10,    3,    11,   // DefensaDer
     -32,  -30,  -42,  -35,  -58,   -1,    2,   -2,  -11,    7,    6,    1,    5,    7,   -10,   // Busqueda
      -8,  -14,   -9,   -2,    5,   -5,    3,   -9,    9,  -12,   -6,   12,  -86,    6,     8,   // EsquivarIzq
      -2,    3,   -5,   -8,    7,    0,    7,   -7,   12,  -18,   12,   -1, -104,    1,   -15,   // EsquivarDer
    // subred 2
      60,   -7,    0,   -5,    9,   -6,    3,   -5,    3,   12,    2,    7,   -1,    5,    -2,   // AtaqueFrontal
      37,    7,   -8,   14,   -2,    7,  -17,   -2,  -17,    9,    0,  -21,  -16,   -1,   -10,   // AjusteIzq
      42,    2,    7,   -1,   -4,  -25,    5,   -6,  -10,  -15,   -3,   -8,  -16,   -3,   -21,   // AjusteDer
     -45,   30,   -1,   -2,    7,    0,   -5,    5,    4,   14,    4,  -10,   -9,    4,   -13,   // CorregirIzq
     -45,   -3,   44,   10,  -12,    2,   -3,   11,   10,   -8,   -3,   -6,   -8,   -4,    -7,   // CorregirDer
     -46,  -40,  -28,   25,   -7,    7,   18,    2,   -9,    1,    4,   12,   -8,    1,     5,   // DefensaIzq
     -23,  -43,  -22,    1,   24,    6,    0,   -8,   -1,  -14,  -16,    2,  -18,   -7,     0,   // DefensaDer
     -26,  -44,  -51,  -44,  -61,   -4,   -1,   -1,   -7,    6,   13,    4,    2,    2,   -23,   // Busqueda
       0,   -4,  -12,    5,   -4,    2,   17,   -5,    4,  -12,  -14,   -8,  -88,    5,     1,   // EsquivarIzq
      -4,   -3,   -6,   -1,    2,  -14,    3,    8,   -3,   -6,   13,    2, -122,    5,     8,   // EsquivarDer
};

}

#endif
