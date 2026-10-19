#include "redes/RedControl.h"
#include "redes/TablaRedControl.h"

namespace redcontrol {
static_assert(NumEntradas == NumEntradasRed && NumSalidas == 2,
              "TablaRedControl.h no coincide con RedControl.h");

namespace {
int limitar(long v) { return v > 255 ? 255 : v < -255 ? -255 : (int)v; }
}

void corregir(const int8_t x[NumEntradasRed], int& avance, int& giro) {
    int16_t oculta[NumOcultas];
    int i = 0;
    for (int j = 0; j < NumOcultas; j++) {
        int suma = LEER_RED(Parametros, i + NumEntradas);
        for (int e = 0; e < NumEntradas; e++) {
            suma += LEER_RED(Parametros, i + e) * x[e];
        }
        oculta[j] = suma > 0 ? suma : 0;
        i += NumEntradas + 1;
    }
    long salida[2];
    for (int k = 0; k < 2; k++) {
        long suma = LEER_RED(Parametros, i + NumOcultas);
        for (int j = 0; j < NumOcultas; j++) {
            suma += (long)LEER_RED(Parametros, i + j) * oculta[j];
        }
        salida[k] = suma;
        i += NumOcultas + 1;
    }
    avance = limitar(salida[0] >> 4);
    giro = limitar(salida[1] >> 4);
}

#if !defined(__AVR__)
int numParametros() { return NumParametros; }
#endif
}
