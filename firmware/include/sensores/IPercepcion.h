#ifndef IPERCEPCION_H
#define IPERCEPCION_H

#include "sensores/LecturasSensores.h"

class IPercepcion {
public:
    virtual ~IPercepcion() {}
    virtual LecturasSensores leer() = 0;
};

#endif
