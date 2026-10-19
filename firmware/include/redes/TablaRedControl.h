#ifndef TABLA_RED_CONTROL_H
#define TABLA_RED_CONTROL_H

#include "redes/TablaRed.h"

namespace redcontrol {
constexpr int NumEntradas = 7;
constexpr int NumOcultas = 6;
constexpr int NumSalidas = 2;
constexpr int NumParametros = NumOcultas * (NumEntradas + 1) + NumSalidas * (NumOcultas + 1);

TABLA_RED(Parametros, NumParametros) = {
       6,   -5,   16,   10,   -4,  -25,    4,     8,
       5,    4,   10,   -5,    7,    5,    2,     2,
     -19,   -9,  -14,  -10,    3,    9,   -4,     5,
       2,   -1,    1,    3,   -4,   -9,   15,     4,
      -6,  -18,   -2,   -9,   -1,    3,   12,    -6,
     -12,    4,   17,    1,  -10,  -16,  -14,    14,
      10,    2,    3,   -2,   -2,   14,   -17,
      -1,   -7,   11,   11,    6,   12,     7,
};
}

#endif
