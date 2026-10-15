#include "estrategia/RedPesos.h"

namespace redpesos {

void entradas(const LecturasSensores& l, int ladoUltimo, int8_t x[NumEntradas],
              bool recienPerdido, bool frontalSostenido) {
    x[0] = l.frontal;
    x[1] = l.c45Izq;
    x[2] = l.c45Der;
    x[3] = l.latIzq;
    x[4] = l.latDer;
    x[5] = ladoUltimo ? 1 : 0;
    x[6] = l.cercaBorde;
    x[7] = recienPerdido;
    x[8] = frontalSostenido;
}

int sumaPonderada(int neurona, const int8_t x[NumEntradas]) {
    int suma = Sesgos[neurona];
    for (int i = 0; i < NumEntradas; i++) {
        suma += x[i] * Pesos[neurona][i];
    }
    return suma;
}

int elegirPorSuma(const int8_t x[NumEntradas], int minima, int16_t* sumas) {
    int mejor = NeuronaBusqueda;
    int mejorSuma = 0;
    for (int n = 0; n < NumNeuronas; n++) {
        const int suma = sumaPonderada(n, x);
        if (sumas) sumas[n] = (int16_t)suma;
        if (n == 0 || suma > mejorSuma) {
            mejor = n;
            mejorSuma = suma;
        }
    }
    // Ninguna neurona se activa lo suficiente: buscar
    return mejorSuma >= minima ? mejor : NeuronaBusqueda;
}

}
