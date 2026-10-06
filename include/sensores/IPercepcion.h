// IPercepcion.h — Interfaz de "lo que ve el robot"
// -----------------------------------------------------------------------------
// El controlador del robot solo pide lecturas a través de esta interfaz, así
// los tests pueden darle lecturas inventadas (mock).
#ifndef IPERCEPCION_H
#define IPERCEPCION_H

#include "sensores/LecturasSensores.h"

class IPercepcion {
public:
    virtual ~IPercepcion() {}
    virtual LecturasSensores leer() = 0;
};

#endif
