// LecturasSensores.h — Foto de todos los sensores en un instante
// Percepcion la rellena en cada ciclo y la estrategia decide a partir de ella.
// Todos los valores ya vienen filtrados: true = hay detección.
#ifndef LECTURAS_SENSORES_H
#define LECTURAS_SENSORES_H

struct LecturasSensores {
    // Sensores de piso: hay línea blanca debajo
    bool lineaIzq;
    bool lineaDer;
    // Algún sensor de piso empieza a ver claro, sin llegar aún a la línea
    bool cercaBorde;
    // Sensores de enemigo: lateral izq, 45° izq, central, 45° der, lateral der
    bool latIzq;
    bool c45Izq;
    bool frontal;
    bool c45Der;
    bool latDer;
};

#endif
