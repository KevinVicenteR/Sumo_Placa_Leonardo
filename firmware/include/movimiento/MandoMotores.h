#ifndef MANDO_MOTORES_H
#define MANDO_MOTORES_H

#include "hardware/IMotor.h"
#include "configuracion/Parametros.h"

struct LimitesAvance {
    int salidaSuave = 255;
    bool cercaBorde = false;
    int estimador = 255;
    bool empujando = false;
};

struct Freno {
    int izq = 0;
    int der = 0;
    unsigned long duracion = 0;
};

class MandoMotores {
public:
    explicit MandoMotores(int rampaPwmPorMs = RampaPwmPorMs): rampa(rampaPwmPorMs) {}

    void nuevoCiclo(unsigned long ahora, bool conRutina);

    void mover(IMotor& motor, int izq, int der, bool urgente = false, bool enSitio = false);

    bool calcularFreno(unsigned long duracionMaxima, Freno& freno) const;

    bool avanzando() const { return estIzq + estDer > 0; }
    int velocidadAvanceEstimada() const { return (estIzq + estDer) / 512; }
    int ordenIzquierda() const { return ordenIzq; }
    int ordenDerecha() const { return ordenDer; }

    LimitesAvance limites;

private:
    int conRampa(int actual, int objetivo) const;
    void estimarVelocidad(unsigned long ahora);

    int rampa;
    int pasoRampa = 255;
    unsigned long ultimaRampa = 0;
    bool primerCiclo = true;
    unsigned long inicioMovimiento = 0;
    int limiteArranque = 255;
    int ordenIzq = 0;
    int ordenDer = 0;
    long estIzq = 0;
    long estDer = 0;
    unsigned long ultimaEstimacion = 0;
};

#endif
