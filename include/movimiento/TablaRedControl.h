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
       6,   -2,    4,    2,   -2,   -2,    9,     4,
       2,   -1,    1,    3,    7,    3,   -3,    -3,
      -6,   -2,   -6,    0,   -1,    1,   -5,    13,
      -3,    2,   -4,   -1,    3,   -3,    5,    -2,
       7,   -2,    1,  -10,    3,    4,    1,    -2,
      -8,    8,    1,   -5,    2,   -3,   -8,     2,
    // salidas
       8,   10,    5,    5,   -3,    5,    -5,   // Avance
      -5,   -3,    6,   13,   -8,    0,     5,   // Giro
};

}

#endif
