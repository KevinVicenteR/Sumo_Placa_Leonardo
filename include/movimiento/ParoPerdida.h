// =============================================================================
// ParoPerdida.h — Frenar en seco al perder al rival en pleno ataque
// -----------------------------------------------------------------------------
// Si el rival desaparece mientras el robot avanza rápido, aplica un contramando
// para no seguir de largo hacia el borde.
// =============================================================================
#ifndef PARO_PERDIDA_H
#define PARO_PERDIDA_H

#include "movimiento/MandoMotores.h"
#include "estrategia/DecisionMovimiento.h"

class ParoPerdida {
public:
    // Un ciclo. Devuelve true mientras esté frenando.
    bool continuar(const DecisionMovimiento& decision, MandoMotores& mando, IMotor& motor,
                   unsigned long ahora);

    // Deja de frenar
    void cancelar() { frenando = false; }
    // Deja de frenar y olvida que venía atacando
    void olvidar() { frenando = false; veniaAtacando = false; }
    // Deja de frenar y considera que viene atacando
    void marcarAtaque() { frenando = false; veniaAtacando = true; }

private:
    bool veniaAtacando = false;
    bool frenando = false;
    unsigned long inicio = 0;
    Freno freno;
};

#endif
