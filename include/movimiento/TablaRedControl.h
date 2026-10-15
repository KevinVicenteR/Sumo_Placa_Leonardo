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
       1,    0,    0,    0,   -2,    0,    1,     1,
       1,    0,   -1,    0,    0,   -1,    1,     0,
      -1,   -1,    1,    1,   -1,    1,   -2,    -1,
      -2,   -1,   -1,    0,   -2,    1,   -1,     0,
       1,    0,   -2,   -1,    0,    0,   -1,     0,
      -2,    1,    0,    2,    0,    0,    0,     1,
    // salidas
       3,    1,    1,    1,    1,    3,     2,   // Avance
      -1,    2,    0,    0,    1,    1,    -1,   // Giro
};

}

#endif
