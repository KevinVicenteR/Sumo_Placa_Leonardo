// TablaRedControl.h — Pesos de la red de control (los escribe simulacion/entrenar_redes.py)
// 7 entradas -> 6 neuronas ReLU -> 2 salidas, enteros int8.
// Cada fila: los pesos de una neurona y, al final, su sesgo. Primero las
// 6 neuronas ocultas (entradas: Error, Cambio, Empujando, TiempoAtaque, BordeDelante, Velocidad, Esquiva) y después las salidas.
#ifndef TABLA_RED_CONTROL_H
#define TABLA_RED_CONTROL_H

#include "estrategia/TablaRed.h"

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
    // salidas
      10,    2,    3,   -2,   -2,   14,   -17,   // Avance
      -1,   -7,   11,   11,    6,   12,     7,   // Giro
};

}

#endif
