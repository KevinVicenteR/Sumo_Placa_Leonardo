// ModuloArranque.h — Filtro de la señal del módulo de arranque (control remoto)
// El módulo da RUN o STOP por un pin. Esta clase filtra el ruido: arrancar
// exige filtroArranqueMs seguidos en RUN y parar exige filtroParadaMs seguidos
// en STOP. El mínimo STOP previo habilita el siguiente flanco de START.
// El modo apaga los motores durante el filtro de parada.
#ifndef MODULO_ARRANQUE_H
#define MODULO_ARRANQUE_H

class ModuloArranque {
public:
    ModuloArranque(unsigned long filtroArranqueMs, unsigned long filtroParadaMs,
                   unsigned long stopMinimoMs = 0)
        : stopMinimoMs(stopMinimoMs), filtroArranqueMs(filtroArranqueMs), filtroParadaMs(filtroParadaMs) {}

    // Recibe el nivel actual del pin y devuelve si el robot debe estar en marcha
    bool enMarcha(bool activo, unsigned long ahora) {
        if (!muestreado || activo != ultimoNivel) cambioDesde = ahora;
        muestreado = true;
        ultimoNivel = activo;
        // El permiso se fija al subir RUN: un pulso rechazado no se acepta
        // aunque dure mucho. Hace falta volver a STOP durante todo el mínimo.
        if (!activo) {
            if (!viendoStop) inicioStop = ahora;
            viendoStop = true;
            autorizado = ahora - inicioStop >= stopMinimoMs;
        } else if (viendoStop) {
            autorizado = ahora - inicioStop >= stopMinimoMs;
            viendoStop = false;
        }
        if (!estable && activo && !autorizado && stopMinimoMs != 0) {
            cambioDesde = ahora;
            return false;
        }
        if (activo == estable) {
            // La señal coincide con el estado actual: reinicia la cuenta
            cambioDesde = ahora;
        } else if (ahora - cambioDesde >= (estable ? filtroParadaMs : filtroArranqueMs)) {
            // La señal lleva suficiente tiempo distinta: cambia de estado
            estable = activo;
            cambioDesde = ahora;
        }
        return estable;
    }

private:
    unsigned long stopMinimoMs;
    unsigned long inicioStop = 0;
    bool viendoStop = false;
    bool autorizado = false;
    unsigned long filtroArranqueMs;
    unsigned long filtroParadaMs;
    unsigned long cambioDesde = 0;
    bool estable = false;
    bool muestreado = false;
    bool ultimoNivel = false;
};

#endif
