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
       0,  -11,    9,    2,    1,   -3,    5,     4,
       5,    1,    6,   -4,    8,    5,   -2,    -5,
      -4,    5,   -3,   -2,   10,    5,   -2,     9,
       0,   -2,   -5,   -5,    0,   -6,    4,    -2,
       4,   -2,   -1,   -8,    2,    4,    1,    -1,
      -3,    4,   -1,   -8,    1,   -1,   -3,     6,
    // salidas
       7,    6,    1,    5,    3,    9,    -2,   // Avance
      -3,   -1,    3,    7,    3,   -2,     2,   // Giro
};

}

#endif
