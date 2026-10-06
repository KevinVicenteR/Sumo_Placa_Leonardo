// ModuloArranque.h — Filtro de la señal del módulo de arranque (control remoto)
// -----------------------------------------------------------------------------
// El módulo da RUN o STOP por un pin. Esta clase filtra el ruido: arrancar
// exige filtroArranqueMs seguidos en RUN y parar exige filtroParadaMs seguidos
// en STOP. La parada usa un filtro más largo para que el infrarrojo del rival
// no detenga (y reinicie) el combate.
#ifndef MODULO_ARRANQUE_H
#define MODULO_ARRANQUE_H

class ModuloArranque {
public:
    ModuloArranque(unsigned long filtroArranqueMs, unsigned long filtroParadaMs)
        : filtroArranqueMs(filtroArranqueMs), filtroParadaMs(filtroParadaMs) {}

    // Recibe el nivel actual del pin y devuelve si el robot debe estar en marcha
    bool enMarcha(bool activo, unsigned long ahora) {
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
    unsigned long filtroArranqueMs;
    unsigned long filtroParadaMs;
    unsigned long cambioDesde = 0;
    bool estable = false;
};

#endif
