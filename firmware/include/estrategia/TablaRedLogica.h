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
      24,   -6,    7,    0,   12,   -7,   -1,   -1,    4,    0,   -1,    6,    -5,
      -3,   23,    6,    0,   -6,  -11,   -3,    1,    7,    4,   -2,    0,    -2,
       6,    4,   27,   -5,  -10,    5,    2,    2,    3,   -8,    9,   -4,     0,
       7,    8,   -9,   20,    2,   -5,   -2,    2,    0,   -3,   11,    7,    15,
       5,   -2,    8,   -4,   20,    3,   -2,   -8,    0,    4,   -3,   -2,    11,
      -9,   -6,    2,    4,   -2,   17,   -6,    6,    8,   -1,    4,   -4,    -1,
       4,    9,    8,    0,   -2,   -5,   15,    5,    0,    3,   -1,    1,    -8,
       2,  -10,    0,   -1,    1,    3,   -3,   15,    3,   -3,   -2,   -5,     6,
      -6,   -7,   -3,   -9,    1,  -10,   -2,    3,   12,    2,    3,    2,    -5,
       1,    5,   -3,   -5,   -6,    0,  -10,   12,    3,   20,    3,   -2,    -3,
      -1,   -5,   -2,   -7,   10,  -10,   11,   -2,    0,    3,   17,   -3,     2,
      -5,   -6,    4,   -1,    1,   -5,   10,   -6,    6,   -3,  -13,   19,     9,
      -9,   -9,   -3,    2,    3,   -2,   -1,   -4,    0,    6,   -6,    4,     4,
       6,   -2,   -4,    3,   -1,   -2,    0,   -7,    3,    0,   -5,   -4,     2,
    // subred 0 (una fila por acción)
      44,    0,    2,   -6,    0,   -2,    8,    0,    1,    6,    7,    3,  -13,    5,     7,   // AtaqueFrontal
      36,    3,  -13,   11,   -3,    1,   -9,   -3,   -5,    8,   -1,   -1,  -24,    3,    -8,   // AjusteIzq
      38,   -5,   13,   -7,   11,   -7,    8,    2,   -4,   -5,   -4,    2,  -24,    4,   -12,   // AjusteDer
     -39,   28,    3,   -3,   -3,   -3,   -1,    4,    3,    5,   -2,   -3,   -8,    9,    -4,   // CorregirIzq
     -48,   -3,   36,    0,  -12,   -3,    2,   11,    2,   -6,   -3,   -8,  -13,   -2,     1,   // CorregirDer
     -45,  -38,  -28,   25,   -1,   -1,    2,   -1,    7,    0,    0,    5,  -12,    8,     4,   // DefensaIzq
     -31,  -45,  -39,    7,   24,    5,    1,   -1,    2,   -9,   -6,    5,   -9,   -5,     3,   // DefensaDer
     -33,  -36,  -39,  -35,  -46,    3,    1,    2,   -3,    2,    4,    7,    3,    4,   -12,   // Busqueda
      -2,    1,   -5,    7,    2,    4,    8,   -1,    6,   -7,   -7,   -4,  -98,    2,     9,   // EsquivarIzq
     -10,   -3,   -7,   -1,    4,    1,    0,    4,   -3,   -9,    1,    0, -110,   -3,     3,   // EsquivarDer
    // selector: una fila por subred (gana la mayor)
       0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,     0,   // subred 0
       0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,     0,   // subred 1
       0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,     0,   // subred 2
    // subred 1
      44,    0,    2,   -6,    0,   -2,    8,    0,    1,    6,    7,    3,  -13,    5,     7,   // AtaqueFrontal
      36,    3,  -13,   11,   -3,    1,   -9,   -3,   -5,    8,   -1,   -1,  -24,    3,    -8,   // AjusteIzq
      38,   -5,   13,   -7,   11,   -7,    8,    2,   -4,   -5,   -4,    2,  -24,    4,   -12,   // AjusteDer
     -39,   28,    3,   -3,   -3,   -3,   -1,    4,    3,    5,   -2,   -3,   -8,    9,    -4,   // CorregirIzq
     -48,   -3,   36,    0,  -12,   -3,    2,   11,    2,   -6,   -3,   -8,  -13,   -2,     1,   // CorregirDer
     -45,  -38,  -28,   25,   -1,   -1,    2,   -1,    7,    0,    0,    5,  -12,    8,     4,   // DefensaIzq
     -31,  -45,  -39,    7,   24,    5,    1,   -1,    2,   -9,   -6,    5,   -9,   -5,     3,   // DefensaDer
     -33,  -36,  -39,  -35,  -46,    3,    1,    2,   -3,    2,    4,    7,    3,    4,   -12,   // Busqueda
      -2,    1,   -5,    7,    2,    4,    8,   -1,    6,   -7,   -7,   -4,  -98,    2,     9,   // EsquivarIzq
     -10,   -3,   -7,   -1,    4,    1,    0,    4,   -3,   -9,    1,    0, -110,   -3,     3,   // EsquivarDer
    // subred 2
      44,    0,    2,   -6,    0,   -2,    8,    0,    1,    6,    7,    3,  -13,    5,     7,   // AtaqueFrontal
      36,    3,  -13,   11,   -3,    1,   -9,   -3,   -5,    8,   -1,   -1,  -24,    3,    -8,   // AjusteIzq
      38,   -5,   13,   -7,   11,   -7,    8,    2,   -4,   -5,   -4,    2,  -24,    4,   -12,   // AjusteDer
     -39,   28,    3,   -3,   -3,   -3,   -1,    4,    3,    5,   -2,   -3,   -8,    9,    -4,   // CorregirIzq
     -48,   -3,   36,    0,  -12,   -3,    2,   11,    2,   -6,   -3,   -8,  -13,   -2,     1,   // CorregirDer
     -45,  -38,  -28,   25,   -1,   -1,    2,   -1,    7,    0,    0,    5,  -12,    8,     4,   // DefensaIzq
     -31,  -45,  -39,    7,   24,    5,    1,   -1,    2,   -9,   -6,    5,   -9,   -5,     3,   // DefensaDer
     -33,  -36,  -39,  -35,  -46,    3,    1,    2,   -3,    2,    4,    7,    3,    4,   -12,   // Busqueda
      -2,    1,   -5,    7,    2,    4,    8,   -1,    6,   -7,   -7,   -4,  -98,    2,     9,   // EsquivarIzq
     -10,   -3,   -7,   -1,    4,    1,    0,    4,   -3,   -9,    1,    0, -110,   -3,     3,   // EsquivarDer
};

}

#endif
