#include "estrategia/RedLogica.h"
#include "estrategia/TablaRedLogica.h"

namespace redlogica {

static_assert(NumEntradas == NumEntradasRed && NumSalidas == NumAcciones,
              "TablaRedLogica.h no coincide con RedLogica.h");

int elegir(const int8_t x[NumEntradasRed], int minima, long* sumas) {
    // Capa oculta: ReLU(sesgo + suma de entrada * peso)
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
    // Salidas: gana la mayor suma
    int mejor = SalidaBusqueda;
    long mejorSuma = 0;
    for (int k = 0; k < NumSalidas; k++) {
        long suma = LEER_RED(Parametros, i + NumOcultas);
        for (int j = 0; j < NumOcultas; j++) {
            suma += (long)LEER_RED(Parametros, i + j) * oculta[j];
        }
        i += NumOcultas + 1;
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
