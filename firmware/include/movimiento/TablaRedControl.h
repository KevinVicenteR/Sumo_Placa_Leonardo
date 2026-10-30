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
      -4,   -1,   10,   -2,    2,    3,    0,     1,
      -1,  -13,   10,  -11,   -6,   -4,    1,    -1,
     -10,   -7,   -5,   -2,   -4,    8,   -6,    21,
      -5,   -4,   -3,    3,  -12,   -9,    4,     5,
       3,  -10,   -3,  -17,    6,    4,    0,    -5,
     -13,    5,   -5,   -7,   -5,    3,   -7,    -6,
    // salidas
      10,   13,    5,    6,   -3,   16,   -10,   // Avance
       2,   -5,   10,   18,   -2,    2,    11,   // Giro
};

}

#endif
