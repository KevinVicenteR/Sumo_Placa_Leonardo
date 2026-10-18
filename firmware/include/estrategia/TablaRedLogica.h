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
      38,  -11,   12,    0,   11,    5,   17,    5,   13,    5,   -2,   -1,    -9,
      -2,   19,   -6,  -15,    3,   -8,    0,   24,    9,   15,   -3,   11,    -3,
     -10,    7,   20,  -11,  -10,   -3,   -3,   -9,  -14,  -13,   -6,    6,     6,
       8,    8,  -10,    4,   -2,   -5,  -18,   -3,   15,  -15,   21,    3,    19,
       7,   -6,    4,   -3,   37,    4,   -6,   -2,  -10,   -1,   -7,    0,     2,
     -16,  -15,    7,   -3,    5,   18,    3,   -1,    0,   16,   -9,  -11,     2,
       9,    2,    4,   -1,   -4,    6,   15,   15,    3,   -1,   -9,  -15,   -23,
      -1,  -18,   11,   -9,   -1,    6,   -6,   37,    0,    2,  -13,   -4,    22,
     -18,   20,    0,   -8,  -15,    4,   -2,    5,   16,   11,    0,  -13,    -1,
      -8,    6,  -18,  -12,    0,   -9,  -12,   15,    0,   24,    0,  -10,    -5,
      -1,  -10,    1,  -13,    6,   -9,   -5,   11,  -18,   -2,   24,    0,    11,
     -10,   -2,    6,    5,   -6,    7,   -4,    3,   -9,   -5,  -13,   24,    -7,
     -11,  -12,    3,    8,   13,   -2,    9,   -8,   17,    0,  -31,   10,     7,
       5,   -7,   -4,    9,    3,    7,  -12,    6,   -7,   12,   -1,    6,    -6,
    // subred 0 (una fila por acción)
      21,   13,   -1,   16,   16,  -14,    8,   -9,  -10,   15,    6,  -12,  -27,   22,     8,   // AtaqueFrontal
      44,    1,  -18,    8,  -11,  -15,  -13,    7,   -6,    3,   -4,   -3,  -16,   -3,    -2,   // AjusteIzq
      28,  -14,   10,    0,   -2,   -7,    2,    2,    8,   -6,   -7,   -1,  -23,    7,     1,   // AjusteDer
     -39,   39,   21,   -5,  -20,    4,   -8,    7,    6,   -4,    9,  -10,   -9,   25,   -12,   // CorregirIzq
     -47,   -8,   40,    1,   -4,    7,   -3,    7,    9,   -7,   -4,  -21,  -16,   -3,    16,   // CorregirDer
     -48,  -53,  -31,   37,   -8,   -6,    3,   -1,   -3,   -3,    5,    1,   10,    1,    -1,   // DefensaIzq
     -33,  -39,  -44,    4,   34,   10,    6,   -1,    9,   -2,   -7,    7,  -27,    3,     5,   // DefensaDer
     -45,  -46,  -38,  -36,  -61,    1,   -2,  -19,  -10,    9,    5,   15,   -5,   15,    -2,   // Busqueda
       3,   10,   -3,    3,    6,   12,  -14,    0,    0,    0,    5,   -8, -100,   -2,     8,   // EsquivarIzq
      -6,    3,    2,    3,    6,   -3,    8,   -2,   -9,  -17,   -2,    4,  -88,    5,     6,   // EsquivarDer
    // selector: una fila por subred (gana la mayor)
     -17,    4,   12,   -5,    3,    9,   -4,    6,    2,    3,   -5,  -22,    0,  -12,    -7,   // subred 0
       5,   -4,   10,   24,    2,    5,   16,    7,  -11,    9,    6,  -14,   -3,  -10,    -7,   // subred 1
      -4,   -3,    9,    1,   -6,    5,   -8,   -7,    2,   11,   -5,    8,    3,    6,    -1,   // subred 2
    // subred 1
      50,    4,    2,   -3,    4,    6,  -11,   -2,    2,   -8,   12,    6,  -20,   -1,     5,   // AtaqueFrontal
      37,   -2,  -14,   -6,    3,   -2,   12,   -2,   18,   11,    9,  -14,  -16,    0,   -23,   // AjusteIzq
      45,  -14,   14,  -20,   -4,  -19,   19,    1,   -5,    1,    2,    7,  -25,   -5,    -4,   // AjusteDer
     -39,   21,    9,    6,   -6,    1,   -9,    0,   -3,    7,    9,    3,  -14,   -6,    -1,   // CorregirIzq
     -48,   12,   45,  -15,    2,   -1,   -8,   12,  -13,   -6,   -5,   -8,   -9,   11,     9,   // CorregirDer
     -61,  -46,  -31,   20,   11,  -17,   -5,    0,    6,  -13,    4,   -2,   -9,    1,    -4,   // DefensaIzq
     -17,  -40,  -42,    8,   20,    6,    5,    7,   -7,  -14,   -2,    5,  -12,  -12,   -14,   // DefensaDer
     -34,  -53,  -48,  -32,  -60,    1,    2,   20,    9,   20,    7,   17,   16,    3,   -14,   // Busqueda
     -12,  -10,  -16,   -2,    4,   11,   13,   -5,   -4,    7,  -11,   -1,  -98,   -4,   -19,   // EsquivarIzq
     -21,   -2,    0,   -7,   -8,    2,    9,    2,   -3,   -8,   -9,   -2, -111,    3,    23,   // EsquivarDer
    // subred 2
      32,   17,    1,    2,  -26,   17,    4,    8,   -4,   20,   -7,   -2,  -17,   10,    -1,   // AtaqueFrontal
      27,   18,   -9,   18,    5,   -3,   -3,    9,  -18,    7,  -14,   -4,  -27,   -2,    -9,   // AjusteIzq
      30,  -14,   20,   -2,    1,  -10,   -2,    8,  -13,  -12,  -15,   12,  -16,    9,   -16,   // AjusteDer
     -39,   31,    1,   -8,  -14,   13,  -13,    8,   -1,    9,   -4,   12,   -7,    8,     3,   // CorregirIzq
     -44,   -6,   34,   -6,   -4,   -9,    1,   13,    2,  -16,   -4,  -11,   -6,   -7,     1,   // CorregirDer
     -56,  -45,  -51,    8,   -8,  -16,   15,   -4,   20,    3,   11,  -14,  -11,  -12,     4,   // DefensaIzq
     -45,  -38,  -31,   -5,   18,    8,    9,   -6,    2,    5,  -15,  -10,   -8,   18,     0,   // DefensaDer
     -56,  -33,  -40,  -45,  -45,   14,    9,   -3,    1,   16,   12,    7,   16,   14,    -8,   // Busqueda
      -3,  -12,    1,   16,   12,   -6,    5,    0,   -3,   20,   -7,   -9, -101,  -14,    12,   // EsquivarIzq
     -15,   -9,   -6,   12,    7,  -18,    4,  -12,    2,   -3,    5,   15,  -78,    2,    12,   // EsquivarDer
};

}

#endif
