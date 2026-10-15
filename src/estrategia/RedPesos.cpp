#include "estrategia/RedPesos.h"

namespace redpesos {

void entradas(const LecturasSensores& l, int ladoUltimo, int8_t x[NumEntradas]) {
    x[0] = l.frontal;
    x[1] = l.c45Izq;
    x[2] = l.c45Der;
    x[3] = l.latIzq;
    x[4] = l.latDer;
    x[5] = ladoUltimo ? 1 : 0;
}

int sumaPonderada(int neurona, const int8_t x[NumEntradas]) {
    int suma = Sesgos[neurona];
    for (int i = 0; i < NumEntradas; i++) {
        suma += x[i] * Pesos[neurona][i];
    }
    return suma;
}

float activar(int suma, int funcion) {
    const float z = (float)suma;
    const float suave = z / (1.0f + (z < 0 ? -z : z));   // -1..1
    switch (funcion) {
    case Relu:
        return z > 0 ? z : 0.0f;
    case Tanh:
        return suave;
    case Sigmoide:
    default:
        return 0.5f * (1.0f + suave);
    }
}

int elegirNeurona(const int8_t x[NumEntradas], int funcion, float umbral, float* activaciones) {
    int mejor = NeuronaBusqueda;
    float mejorActivacion = 0;
    for (int n = 0; n < NumNeuronas; n++) {
        const float a = activar(sumaPonderada(n, x), funcion);
        if (activaciones) activaciones[n] = a;
        if (n == 0 || a > mejorActivacion) {
            mejor = n;
            mejorActivacion = a;
        }
    }
    // Ninguna neurona se activa lo suficiente: buscar
    return mejorActivacion >= umbral ? mejor : NeuronaBusqueda;
}

}
