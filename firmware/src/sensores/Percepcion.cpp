#include <Arduino.h>
#include "sensores/Percepcion.h"

void Percepcion::calibrarPiso() {
    pisoIzq.calibrar();
    pisoDer.calibrar();
}

void Percepcion::calibrarSensoresEnemigo() {
    SensorEnemigo* sensores[] = {&latIzq, &c45Izq, &frontal, &c45Der, &latDer};
    for (SensorEnemigo* sensor : sensores) {
        if (PolaridadSensoresEnemigo != 0) {
            sensor->fijarPolaridad(PolaridadSensoresEnemigo > 0);
        } else {
            sensor->aprenderPolaridad();
        }
    }
    for (SensorEnemigo* sensor : sensores) {
        sensor->reiniciarFiltro();
    }
}

LecturasSensores Percepcion::leer() {
    const unsigned long ahora = millis();
    LecturasSensores lecturas;

    // Piso: se leen los dos sensores y después se interpretan
    const int izq = pisoIzq.leer();
    const int der = pisoDer.leer();
    lecturas.lineaIzq = pisoIzq.hayLinea(izq);
    lecturas.lineaDer = pisoDer.hayLinea(der);
    lecturas.cercaBorde = pisoIzq.cercaDeLinea(izq) || pisoDer.cercaDeLinea(der);

    // Enemigo: cada sensor ya devuelve la lectura filtrada
    lecturas.latIzq = latIzq.detecta(ahora);
    lecturas.c45Izq = c45Izq.detecta(ahora);
    lecturas.frontal = frontal.detecta(ahora);
    lecturas.c45Der = c45Der.detecta(ahora);
    lecturas.latDer = latDer.detecta(ahora);
    return lecturas;
}
