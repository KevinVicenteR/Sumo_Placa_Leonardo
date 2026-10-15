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
      17,    0,    1,    0,    0,    0,    0,   -1,    0,   -1,    0,   -1,     0,
       0,   17,    1,   -1,   -1,    0,    1,    1,    1,    0,    0,    0,    -1,
       1,    1,   15,   -1,    0,   -1,    0,    0,   -1,    0,   -1,    0,    -1,
      -1,   -1,    1,   17,    0,    0,   -1,   -1,   -1,    0,    1,    2,     1,
       0,   -1,    0,    0,   16,    2,    0,   -1,   -1,    0,    1,    0,     0,
       0,    0,   -1,    0,   -1,   15,    0,    1,    0,    0,    0,   -1,     0,
       0,    0,    1,    0,    0,   -2,   15,    0,    0,    1,    1,    0,     1,
       0,    0,    0,   -1,   -1,    0,    0,   16,    0,    1,    0,   -1,    -1,
       0,   -1,    0,   -1,   -1,    0,   -1,   -1,   17,    0,    0,   -1,     0,
       0,    0,    1,   -1,   -1,    0,    0,    1,    0,   16,    1,   -1,    -1,
      -1,   -1,    2,   -1,    1,    1,    0,    1,    0,    0,   16,    1,     0,
       1,   -1,   -1,    0,    0,    1,    0,    0,    0,    0,    0,   16,     0,
       0,    0,   -2,    0,    1,    0,    0,    0,    1,    0,   -1,    1,    15,
       1,    0,    0,    0,    0,    0,    0,   -1,    0,    0,   -1,   -1,     0,
    // salidas
      40,    4,    3,    0,    0,    0,    1,    0,    0,    1,    0,    0,  -21,    0,     0,   // AtaqueFrontal
      40,   12,  -14,    1,    0,   -1,   -1,   -1,    0,    1,    1,    0,  -24,    1,     0,   // AjusteIzq
      40,  -12,   13,    0,    1,   -1,    0,    0,    0,    0,    0,    0,  -24,   -1,     0,   // AjusteDer
     -40,   33,    0,   -1,    0,    0,    1,   -1,   -1,    0,    0,    0,   -8,    0,     1,   // CorregirIzq
     -41,   -4,   31,   -1,   -1,   -1,    0,    1,    0,   -1,    0,   -1,   -9,    1,     0,   // CorregirDer
     -40,  -40,  -40,   16,    1,   -1,    1,    0,    1,    0,   -1,    1,   -3,    0,     0,   // DefensaIzq
     -40,  -40,  -41,   -4,   16,    0,    0,    1,    1,    0,   -1,    0,   -4,    0,     0,   // DefensaDer
     -40,  -39,  -40,  -40,  -41,    0,    1,    0,    0,    1,    0,    0,    4,    1,     0,   // Busqueda
       0,    0,    0,    0,    0,    0,    1,    0,    0,    0,    0,    0, -100,    0,     0,   // EsquivarIzq
      -1,    1,   -1,    1,   -1,    1,    0,    0,    0,   -1,   -1,    0, -102,    0,    -1,   // EsquivarDer
};

}

#endif
