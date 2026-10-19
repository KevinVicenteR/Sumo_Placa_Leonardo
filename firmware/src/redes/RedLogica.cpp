#include "redes/RedLogica.h"
#include "redes/TablaRedLogica.h"

namespace redlogica {
static_assert(NumEntradas == NumEntradasRed && NumSalidas == NumAcciones && NumOcultas == NumOcultasRed &&
              NumSubredes == NumSubredesRed,
              "TablaRedLogica.h no coincide con RedLogica.h");

namespace {
int ultimaSubred = 0;
long neurona(int i, const int oculta[NumOcultas]) {
    long suma = LEER_RED(Parametros, i + NumOcultas);
    for (int j = 0; j < NumOcultas; j++) {
        suma += (long)LEER_RED(Parametros, i + j) * oculta[j];
    }
    return suma;
}
}

int subredElegida() { return ultimaSubred; }

int elegir(const int8_t x[NumEntradasRed], int minima, long* sumas) {
    int oculta[NumOcultas];
    int i = 0;
    for (int j = 0; j < NumOcultas; j++) {
        int suma = 0;
        for (int e = 0; e < NumEntradas; e++) {
            if (x[e]) suma += LEER_RED(Parametros, i + e) * x[e];
        }
        suma += LEER_RED(Parametros, i + NumEntradas);
        oculta[j] = suma > 0 ? suma : 0;
        i += NumEntradas + 1;
    }
    constexpr int Fila = NumOcultas + 1;
    constexpr int Subred = NumSalidas * Fila;
    const int inicioSubred0 = i;
    const int inicioSelector = i + Subred;
    int elegida = 0;
    long mejorSelector = 0;
    for (int k = 0; k < NumSubredes; k++) {
        const long s = neurona(inicioSelector + k * Fila, oculta);
        if (k == 0 || s > mejorSelector) {
            elegida = k;
            mejorSelector = s;
        }
    }
    ultimaSubred = elegida;
    i = elegida == 0 ? inicioSubred0 : inicioSelector + NumSubredes * Fila + (elegida - 1) * Subred;
    int mejor = SalidaBusqueda;
    long mejorSuma = 0;
    for (int k = 0; k < NumSalidas; k++) {
        const long suma = neurona(i, oculta);
        i += Fila;
        if (sumas) sumas[k] = suma;
        if (k == 0 || suma > mejorSuma) {
            mejor = k;
            mejorSuma = suma;
        }
    }
    return mejorSuma >= minima ? mejor : SalidaBusqueda;
}

#if !defined(__AVR__)
int numParametros() { return NumParametros; }
#endif
}
