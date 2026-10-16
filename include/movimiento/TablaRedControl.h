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
       5,   -9,    4,    3,    2,   -3,    4,     4,
       4,   -1,    3,    1,    8,    6,   -6,    -4,
      -6,    5,   -3,    2,    6,    3,   -2,     5,
      -3,   -2,   -9,   -4,   -3,   -2,    7,    -2,
       4,   -1,    0,  -12,    4,    6,    1,    -1,
      -4,    6,    3,   -4,    2,   -6,   -4,     7,
    // salidas
      10,    5,    6,    2,   -3,    5,    -4,   // Avance
      -4,   -4,    6,    3,    0,    2,     2,   // Giro
};

}

#endif
