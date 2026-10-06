// =============================================================================
// ModoPruebaMotores.h — Comprobar el sentido de cada rueda
// -----------------------------------------------------------------------------
// Se compila con: pio run -e prueba_motores -t upload -t monitor
// Con el robot levantado (ruedas en el aire) repite una secuencia de
// movimientos y anuncia cada uno por USB.
// =============================================================================
#ifndef MODO_PRUEBA_MOTORES_H
#define MODO_PRUEBA_MOTORES_H

#include "hardware/DriverMotores.h"
#include "hardware/PlacaXMotion.h"

class ModoPruebaMotores {
public:
    ModoPruebaMotores(PlacaXMotion& placa, DriverMotores& motores): placa(placa), motores(motores) {}

    void iniciar();
    void actualizar();

private:
    // Anuncia el paso, mueve 1,5 s y para 1 s
    void paso(const __FlashStringHelper* texto, int izq, int der);

    PlacaXMotion& placa;
    DriverMotores& motores;
};

#endif
