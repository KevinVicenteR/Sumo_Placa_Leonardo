#ifndef MODULO_ARRANQUE_H
#define MODULO_ARRANQUE_H

class ModuloArranque {
public:
    ModuloArranque(unsigned long filtroArranqueMs, unsigned long filtroParadaMs,
                   unsigned long stopMinimoMs = 0)
        : stopMinimoMs(stopMinimoMs), filtroArranqueMs(filtroArranqueMs), filtroParadaMs(filtroParadaMs) {}

    bool enMarcha(bool activo, unsigned long ahora) {
        if (!muestreado || activo != ultimoNivel) cambioDesde = ahora;
        muestreado = true;
        ultimoNivel = activo;
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
            cambioDesde = ahora;
        } else if (ahora - cambioDesde >= (estable ? filtroParadaMs : filtroArranqueMs)) {
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
