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
      23,   -9,    8,   -1,   11,   -4,    4,    1,    0,    7,    0,    1,    -4,
      -3,   22,    3,    3,   -2,   -9,    4,    5,    6,    7,   -3,    4,    -5,
       4,    1,   22,   -3,   -6,    3,   -3,   -1,    2,   -4,    3,    0,     1,
       4,    2,   -6,   16,    4,    0,   -4,   -2,    2,   -4,    6,    3,    15,
       7,    2,    4,   -1,   21,    3,    3,    1,   -2,    1,   -5,    1,     4,
      -8,  -12,   -1,    4,    3,   19,   -5,    0,    2,    2,    2,   -4,    -2,
      -1,    8,    5,   -5,   -3,    1,   13,    3,   -1,    3,    2,   -2,    -9,
      -1,   -9,   -1,    0,   -3,   -1,   -5,   20,    8,   -1,    0,   -5,     0,
      -8,    3,    1,   -3,    1,   -7,    2,    4,   12,    1,    0,   -9,    -3,
      -1,   10,   -3,   -4,   -2,   -5,   -5,    4,    4,   20,   -3,   -3,     0,
      -1,   -2,    2,  -11,   12,  -14,    7,    0,   -3,    5,   14,    0,     2,
      -9,   -5,    0,    3,    3,    3,   -2,   -3,   -1,    0,   -8,   21,     6,
      -7,    2,   -8,   -1,    3,    0,    2,   -5,    3,    5,   -5,    0,     1,
       5,   -3,    0,    3,   -5,    1,   -3,   -7,    2,    1,    4,    1,    -4,
    // subred 0 (una fila por acción)
      44,    3,    0,    1,    1,    1,    4,   -3,    3,    6,    4,    4,  -15,    3,     0,   // AtaqueFrontal
      32,    8,  -13,    8,    4,   -7,   -8,   -1,   -4,   10,   -6,    2,  -22,   -1,    -7,   // AjusteIzq
      33,  -10,   10,   -8,    3,  -10,    8,    0,   -5,   -6,   -3,    1,  -26,   -3,    -6,   // AjusteDer
     -44,   26,    4,    2,   -4,   -1,   -7,    1,    1,    6,   -2,   -1,   -2,    5,    -5,   // CorregirIzq
     -51,   -1,   41,   -1,   -7,   -5,   -2,   12,   -2,   -6,    2,   -6,  -13,   -2,     4,   // CorregirDer
     -47,  -41,  -30,   23,   -2,   -4,    2,   -1,    6,   -2,   -1,    1,   -5,    0,     1,   // DefensaIzq
     -31,  -42,  -39,    2,   26,    4,    1,    2,   -1,   -5,   -7,    1,   -5,   -5,     1,   // DefensaDer
     -38,  -38,  -42,  -38,  -48,    2,   -5,   -1,    2,    9,    7,   11,    8,    7,    -7,   // Busqueda
       1,   -1,   -1,    0,    3,    3,    1,    0,    5,   -4,   -5,   -6,  -97,    0,     0,   // EsquivarIzq
      -9,   -6,   -5,    0,    2,    1,    0,    3,   -6,   -6,    0,    1, -103,    3,     4,   // EsquivarDer
    // selector: una fila por subred (gana la mayor)
       0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,     0,   // subred 0
       0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,     0,   // subred 1
       0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,     0,   // subred 2
    // subred 1
      44,    3,    0,    1,    1,    1,    4,   -3,    3,    6,    4,    4,  -15,    3,     0,   // AtaqueFrontal
      32,    8,  -13,    8,    4,   -7,   -8,   -1,   -4,   10,   -6,    2,  -22,   -1,    -7,   // AjusteIzq
      33,  -10,   10,   -8,    3,  -10,    8,    0,   -5,   -6,   -3,    1,  -26,   -3,    -6,   // AjusteDer
     -44,   26,    4,    2,   -4,   -1,   -7,    1,    1,    6,   -2,   -1,   -2,    5,    -5,   // CorregirIzq
     -51,   -1,   41,   -1,   -7,   -5,   -2,   12,   -2,   -6,    2,   -6,  -13,   -2,     4,   // CorregirDer
     -47,  -41,  -30,   23,   -2,   -4,    2,   -1,    6,   -2,   -1,    1,   -5,    0,     1,   // DefensaIzq
     -31,  -42,  -39,    2,   26,    4,    1,    2,   -1,   -5,   -7,    1,   -5,   -5,     1,   // DefensaDer
     -38,  -38,  -42,  -38,  -48,    2,   -5,   -1,    2,    9,    7,   11,    8,    7,    -7,   // Busqueda
       1,   -1,   -1,    0,    3,    3,    1,    0,    5,   -4,   -5,   -6,  -97,    0,     0,   // EsquivarIzq
      -9,   -6,   -5,    0,    2,    1,    0,    3,   -6,   -6,    0,    1, -103,    3,     4,   // EsquivarDer
    // subred 2
      44,    3,    0,    1,    1,    1,    4,   -3,    3,    6,    4,    4,  -15,    3,     0,   // AtaqueFrontal
      32,    8,  -13,    8,    4,   -7,   -8,   -1,   -4,   10,   -6,    2,  -22,   -1,    -7,   // AjusteIzq
      33,  -10,   10,   -8,    3,  -10,    8,    0,   -5,   -6,   -3,    1,  -26,   -3,    -6,   // AjusteDer
     -44,   26,    4,    2,   -4,   -1,   -7,    1,    1,    6,   -2,   -1,   -2,    5,    -5,   // CorregirIzq
     -51,   -1,   41,   -1,   -7,   -5,   -2,   12,   -2,   -6,    2,   -6,  -13,   -2,     4,   // CorregirDer
     -47,  -41,  -30,   23,   -2,   -4,    2,   -1,    6,   -2,   -1,    1,   -5,    0,     1,   // DefensaIzq
     -31,  -42,  -39,    2,   26,    4,    1,    2,   -1,   -5,   -7,    1,   -5,   -5,     1,   // DefensaDer
     -38,  -38,  -42,  -38,  -48,    2,   -5,   -1,    2,    9,    7,   11,    8,    7,    -7,   // Busqueda
       1,   -1,   -1,    0,    3,    3,    1,    0,    5,   -4,   -5,   -6,  -97,    0,     0,   // EsquivarIzq
      -9,   -6,   -5,    0,    2,    1,    0,    3,   -6,   -6,    0,    1, -103,    3,     4,   // EsquivarDer
};

}

#endif
