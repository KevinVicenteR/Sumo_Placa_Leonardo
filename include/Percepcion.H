#ifndef PERCEPCION_H
#define PERCEPCION_H

#include <stdint.h>
#include "LecturasSensores.H"
#include "IPercepcion.H"
#include "RetencionDeteccion.H"
#include "Parametros.H"

class Percepcion: public IPercepcion {
public:
    LecturasSensores leer() const override;

private:
    static bool lecturaDigitalMayoritaria(uint8_t pin);

    mutable RetencionDeteccion retLatIzq{TiempoRetencionEnemigo};
    mutable RetencionDeteccion retC45Izq{TiempoRetencionEnemigo};
    mutable RetencionDeteccion retFrontal{TiempoRetencionEnemigo};
    mutable RetencionDeteccion retC45Der{TiempoRetencionEnemigo};
    mutable RetencionDeteccion retLatDer{TiempoRetencionEnemigo};
};

#endif
