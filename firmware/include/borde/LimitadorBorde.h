#ifndef LIMITADOR_BORDE_H
#define LIMITADOR_BORDE_H

#include "borde/EstimadorBorde.h"
#include "movimiento/MandoMotores.h"
#include "estrategia/DecisionMovimiento.h"

class LimitadorBorde {
public:
    void actualizar(const DecisionMovimiento& decision, const MandoMotores& mando, bool empujando,
                    unsigned long ahora);
    void colocarSalida(int rutina);

    int limite() const { return limiteAvance; }
    const EstimadorBorde& estimador() const { return estimadorBorde; }

private:
    EstimadorBorde estimadorBorde;
    unsigned long ultimaPrediccion = 0;
    bool bordeAntes = false;
    int limiteAvance = 255;
};

#endif
