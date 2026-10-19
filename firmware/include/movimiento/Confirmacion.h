#ifndef CONFIRMACION_H
#define CONFIRMACION_H

#include "configuracion/Parametros.h"

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
