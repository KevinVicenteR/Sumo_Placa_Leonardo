#ifndef LECTURAS_SENSORES_H
#define LECTURAS_SENSORES_H

struct LecturasSensores {
    bool lineaIzq;
    bool lineaDer;
    // Algún sensor de piso empieza a ver claro, sin llegar aún al umbral de línea
    bool cercaBorde;
    bool latIzq;
    bool c45Izq;
    bool frontal;
    bool c45Der;
    bool latDer;
};

#endif
