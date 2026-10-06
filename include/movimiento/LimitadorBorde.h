// LimitadorBorde.h — Limita la velocidad según el estimador de borde
// Alimenta a EstimadorBorde con las órdenes de motor de cada ciclo y con cada
// línea vista, y traduce su predicción en una velocidad máxima de avance.
#ifndef LIMITADOR_BORDE_H
#define LIMITADOR_BORDE_H

#include "movimiento/EstimadorBorde.h"
#include "movimiento/MandoMotores.h"
#include "estrategia/DecisionMovimiento.h"

class LimitadorBorde {
public:
    void actualizar(const DecisionMovimiento& decision, const MandoMotores& mando, bool empujando,
                    unsigned long ahora);

    // Velocidad máxima de avance (255 = sin límite)
    int limite() const { return limiteAvance; }
    const EstimadorBorde& estimador() const { return estimadorBorde; }

private:
    EstimadorBorde estimadorBorde;
    unsigned long ultimaPrediccion = 0;
    bool bordeAntes = false;
    int limiteAvance = 255;
};

#endif
