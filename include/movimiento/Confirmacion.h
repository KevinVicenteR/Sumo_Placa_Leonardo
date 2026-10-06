// =============================================================================
// Confirmacion.h — Exige que una detección dure un poco antes de creerla
// -----------------------------------------------------------------------------
// Devuelve true solo cuando la detección lleva ConfirmacionDeteccion ms
// seguidos. Evita que un reflejo o un parpadeo termine una maniobra.
// =============================================================================
#ifndef CONFIRMACION_H
#define CONFIRMACION_H

#include "Parametros.h"

class Confirmacion {
public:
    bool actualizar(bool detectando, unsigned long ahora) {
        if (!detectando) {
            activa = false;
            return false;
        }
        if (!activa) {
            activa = true;
            inicio = ahora;
        }
        return ahora - inicio >= ConfirmacionDeteccion;
    }

    void reiniciar() { activa = false; }

private:
    bool activa = false;
    unsigned long inicio = 0;
};

#endif
