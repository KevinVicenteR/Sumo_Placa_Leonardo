// TablaRedLogica.h — Pesos de la red de lógica (los escribe simulacion/entrenar_redes.py)
// 12 entradas -> 14 neuronas ReLU -> 10 salidas, enteros int8.
// Cada fila: los pesos de una neurona y, al final, su sesgo. Primero las
// 14 neuronas ocultas (entradas: Fr, 45I, 45D, LatI, LatD, Lado, Borde, Perd, Emp, BordeDelante, Esquivando, Rutina) y después las salidas.
#ifndef TABLA_RED_LOGICA_H
#define TABLA_RED_LOGICA_H

#include "estrategia/TablaRed.h"

namespace redlogica {

constexpr int NumEntradas = 12;
constexpr int NumOcultas = 14;
constexpr int NumSalidas = 10;
constexpr int NumParametros = NumOcultas * (NumEntradas + 1) + NumSalidas * (NumOcultas + 1);

TABLA_RED(Parametros, NumParametros) = {
      17,    0,    1,    0,    0,    0,   -1,    0,    0,   -1,   -1,    0,     0,
       1,   16,    1,   -2,   -2,   -1,    0,    2,    1,    1,    0,   -1,     0,
       1,    3,   15,    1,    0,    0,   -1,   -1,   -2,    0,   -3,    2,    -2,
      -1,   -1,    1,   19,    0,    0,   -1,    0,   -2,   -1,    1,    2,     1,
      -1,   -2,    0,    0,   16,    3,   -1,   -1,    0,    1,    1,   -1,     0,
       0,    1,   -1,    0,   -1,   16,   -1,    2,    0,   -1,   -1,   -2,     0,
      -1,    0,    0,    0,    1,   -3,   13,    0,    1,    1,    1,    0,     1,
      -1,    0,    0,   -1,   -1,   -1,   -1,   15,    0,    0,    1,   -1,     0,
       0,   -1,    0,   -3,   -1,    0,   -1,   -1,   18,   -1,    1,   -2,     0,
       1,    1,    0,   -1,   -2,   -1,   -2,   -1,   -1,   15,    1,    0,     1,
       0,    0,    1,   -2,    1,    1,    0,    1,   -1,    0,   16,    1,     1,
       0,    0,   -1,    0,    1,    1,    2,    0,   -1,    1,    0,   16,     1,
       0,    1,   -2,    1,    1,    0,    0,   -1,    1,    1,   -1,    2,    15,
       1,    0,    0,    0,   -1,    0,    0,   -1,    1,    0,   -1,   -1,    -1,
    // salidas
      38,    5,    2,    1,    1,   -1,    1,    0,    0,   -1,   -1,   -1,  -20,    0,    -1,   // AtaqueFrontal
      40,   11,  -14,    0,    1,   -2,   -1,    0,    1,    2,    1,   -1,  -23,    0,     0,   // AjusteIzq
      39,  -10,   14,    0,    0,    0,    0,    0,    0,   -1,    1,    1,  -24,   -1,    -1,   // AjusteDer
     -41,   32,    1,   -1,    0,    0,    1,   -1,   -1,    1,    1,    0,   -9,    0,     0,   // CorregirIzq
     -41,   -3,   31,   -2,   -3,   -2,    0,    3,    0,   -2,    1,   -1,   -7,    1,     0,   // CorregirDer
     -40,  -39,  -39,   17,    1,    0,    1,    0,    0,    0,    0,    1,   -2,   -2,     0,   // DefensaIzq
     -40,  -41,  -41,   -3,   16,    0,    0,    1,    1,    0,    0,   -1,   -4,   -1,     1,   // DefensaDer
     -40,  -39,  -40,  -41,  -41,   -1,    1,    0,    0,    2,    1,    0,    5,    1,     0,   // Busqueda
       0,    0,   -1,    0,    0,    0,    2,    0,    1,    0,    0,    0, -100,    0,     0,   // EsquivarIzq
      -2,    1,    0,    1,   -1,    0,    1,    0,    0,    0,    0,   -1, -103,   -1,    -1,   // EsquivarDer
};

}

#endif
