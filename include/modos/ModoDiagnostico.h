// ModoDiagnostico.h — Revisar los sensores con los motores apagados
// Se compila con: pio run -e diagnostico -t upload -t monitor
// Tras TiempoInicioReglamentario calibra los sensores e imprime por USB todas
// las lecturas cada 100 ms. Por el monitor serie: 'c' recalibra, 'r' muestra la
// caja negra (últimos combates) y 'b' la borra.
#ifndef MODO_DIAGNOSTICO_H
#define MODO_DIAGNOSTICO_H

#include "hardware/DriverMotores.h"
#include "hardware/PlacaXMotion.h"
#include "sensores/Percepcion.h"
#include "hardware/CajaNegra.h"

class ModoDiagnostico {
public:
    ModoDiagnostico(PlacaXMotion& placa, DriverMotores& motores, Percepcion& percepcion)
        : placa(placa), motores(motores), percepcion(percepcion) {}

    void iniciar();
    void actualizar();

private:
    void calibrar();
    void imprimirCajaNegra();

    PlacaXMotion& placa;
    DriverMotores& motores;
    Percepcion& percepcion;
    CajaNegra cajaNegra;
};

#endif
