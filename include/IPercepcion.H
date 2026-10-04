#ifndef IPERCEPCION_H
#define IPERCEPCION_H

#include "LecturasSensores.H"

class IPercepcion {
public:
    virtual ~IPercepcion() {}
    virtual LecturasSensores leer() const = 0;
    // Comprobación rápida opcional justo antes de activar los motores.
    virtual LecturasSensores leerBorde() const { return LecturasSensores{}; }
};

#endif
