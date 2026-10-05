#ifndef MODULO_ARRANQUE_H
#define MODULO_ARRANQUE_H

class ModuloArranque {
public:
    explicit ModuloArranque(unsigned long filtroMs): filtroMs(filtroMs) {}

    bool enMarcha(bool activo, unsigned long ahora) {
        if (activo == estable) {
            cambioDesde = ahora;
        } else if (ahora - cambioDesde >= filtroMs) {
            estable = activo;
            cambioDesde = ahora;
        }
        return estable;
    }

private:
    unsigned long filtroMs;
    unsigned long cambioDesde = 0;
    bool estable = false;
};

#endif
