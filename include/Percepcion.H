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
    // Debe llamarse con el robot sobre el negro del dohyo
    void calibrarPiso();

private:
    static bool lecturaDigitalMayoritaria(uint8_t pin);
    static int promedioAnalogico(uint8_t pin, int muestras);

    int umbralIzq = 0;
    int umbralDer = 0;

    mutable RetencionDeteccion retLatIzq{TiempoRetencionEnemigo};
    mutable RetencionDeteccion retC45Izq{TiempoRetencionEnemigo};
    mutable RetencionDeteccion retFrontal{TiempoRetencionEnemigo};
    mutable RetencionDeteccion retC45Der{TiempoRetencionEnemigo};
    mutable RetencionDeteccion retLatDer{TiempoRetencionEnemigo};
};

#endif
