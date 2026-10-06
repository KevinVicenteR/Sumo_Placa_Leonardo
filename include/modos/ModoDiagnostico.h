// =============================================================================
// ModoDiagnostico.h — Revisar los sensores con los motores apagados
// -----------------------------------------------------------------------------
// Se compila con: pio run -e diagnostico -t upload -t monitor
// Tras TiempoInicioReglamentario calibra los sensores e imprime por USB todas
// las lecturas cada 100 ms. Enviando 'c' por el monitor serie recalibra.
// =============================================================================
#ifndef MODO_DIAGNOSTICO_H
#define MODO_DIAGNOSTICO_H

#include "hardware/DriverMotores.h"
#include "hardware/PlacaXMotion.h"
#include "sensores/Percepcion.h"

class ModoDiagnostico {
public:
    ModoDiagnostico(PlacaXMotion& placa, DriverMotores& motores, Percepcion& percepcion)
        : placa(placa), motores(motores), percepcion(percepcion) {}

    void iniciar();
    void actualizar();

private:
    void calibrar();

    PlacaXMotion& placa;
    DriverMotores& motores;
    Percepcion& percepcion;
};

#endif
