#ifndef SENSOR_ENEMIGO_H
#define SENSOR_ENEMIGO_H

#include <stdint.h>
#include "sensores/RetencionDeteccion.h"

class SensorEnemigo {
public:
    SensorEnemigo(uint8_t pin, bool activoAlto);

    bool detecta(unsigned long ahora);

    void reiniciarFiltro();

private:
    bool mayoriaAlta() const;

    uint8_t pin;
    bool activoAlto;
    RetencionDeteccion filtro;
};

#endif
