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
       4,   -8,    6,    1,    2,   -2,    2,     4,
       3,   -2,    4,    1,    8,    5,   -2,    -3,
      -3,    4,   -3,    1,    8,    3,   -2,     6,
      -2,   -1,   -8,   -4,   -1,   -3,    6,    -2,
       3,   -1,    0,   -8,    2,    4,    2,    -2,
      -3,    5,    2,   -3,    1,   -3,   -4,     5,
    // salidas
       8,    5,    4,    2,   -3,    5,    -5,   // Avance
      -1,   -1,    3,    5,    0,    1,     0,   // Giro
};

}

#endif
