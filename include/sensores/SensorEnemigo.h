// =============================================================================
// SensorEnemigo.h — Un sensor digital que detecta al robot rival
// -----------------------------------------------------------------------------
// Cada lectura toma 3 muestras y se queda con la mayoría (filtra parpadeos) y
// después pasa por un filtro de retención (RetencionDeteccion).
// La polaridad indica si el sensor da HIGH o LOW cuando ve algo.
// =============================================================================
#ifndef SENSOR_ENEMIGO_H
#define SENSOR_ENEMIGO_H

#include <stdint.h>
#include "sensores/RetencionDeteccion.h"

class SensorEnemigo {
public:
    SensorEnemigo(uint8_t pin, bool activoAlto);

    // Lectura filtrada: true = ve al rival
    bool detecta(unsigned long ahora);

    // Fija la polaridad: true = detecta en HIGH
    void fijarPolaridad(bool activoAltoNuevo) { activoAlto = activoAltoNuevo; }
    // Aprende la polaridad midiendo SIN nada delante: detectar es cambiar de nivel
    void aprenderPolaridad();
    // Olvida las detecciones anteriores
    void reiniciarFiltro();

private:
    // El pin está en HIGH en al menos 2 de 3 lecturas
    bool mayoriaAlta() const;

    uint8_t pin;
    bool activoAlto;
    RetencionDeteccion filtro;
};

#endif
