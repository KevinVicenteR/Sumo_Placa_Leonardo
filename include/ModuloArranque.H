#ifndef MODULO_ARRANQUE_H
#define MODULO_ARRANQUE_H

class ModuloArranque {
public:
    ModuloArranque(unsigned long filtroArranqueMs, unsigned long filtroParadaMs)
        : filtroArranqueMs(filtroArranqueMs), filtroParadaMs(filtroParadaMs) {}

    bool enMarcha(bool activo, unsigned long ahora) {
        if (activo == estable) {
            cambioDesde = ahora;
        } else if (ahora - cambioDesde >= (estable ? filtroParadaMs : filtroArranqueMs)) {
            estable = activo;
            cambioDesde = ahora;
        }
        return estable;
    }

private:
    unsigned long filtroArranqueMs;
    unsigned long filtroParadaMs;
    unsigned long cambioDesde = 0;
    bool estable = false;
};

#endif
