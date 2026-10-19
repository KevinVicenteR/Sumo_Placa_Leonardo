#include <Arduino.h>
#include "sensores/Percepcion.h"

void Percepcion::calibrarPiso() {
    pisoIzq.calibrar();
    pisoDer.calibrar();
}

LecturasSensores Percepcion::leer() {
    const unsigned long ahora = millis();
    LecturasSensores lecturas;

    const int izq = pisoIzq.leer();
    const int der = pisoDer.leer();
    lecturas.lineaIzq = pisoIzq.hayLinea(izq);
    lecturas.lineaDer = pisoDer.hayLinea(der);
    lecturas.cercaBorde = pisoIzq.cercaDeLinea(izq) || pisoDer.cercaDeLinea(der);

    lecturas.latIzq = latIzq.detecta(ahora);
    lecturas.c45Izq = c45Izq.detecta(ahora);
    lecturas.frontal = frontal.detecta(ahora);
    lecturas.c45Der = c45Der.detecta(ahora);
    lecturas.latDer = latDer.detecta(ahora);
    return lecturas;
}
