// =============================================================================
// GiroLateral.h — Girar hacia un rival visto por un sensor lateral
// -----------------------------------------------------------------------------
// Una vez empezado, el giro sigue aunque el sensor lateral deje de verlo, hasta
// tener al rival de frente (o por el otro lado), o hasta TiempoMaxGiroLateral.
// =============================================================================
#ifndef GIRO_LATERAL_H
#define GIRO_LATERAL_H

#include "movimiento/MandoMotores.h"
#include "movimiento/Confirmacion.h"
#include "estrategia/DecisionMovimiento.h"

class GiroLateral {
public:
    // Un ciclo del giro. Devuelve true mientras siga girando.
    bool continuar(const DecisionMovimiento& decision, MandoMotores& mando, IMotor& motor,
                   unsigned long ahora);
    void cancelar() { sentido = 0; }

    // Gira hacia un lado (1 = derecha, -1 = izquierda), pivotando sobre una
    // rueda o en el sitio según GiroLateralEnRueda
    static void girarHacia(MandoMotores& mando, IMotor& motor, int sentido);

private:
    // 0 = sin giro en marcha
    int sentido = 0;
    unsigned long inicio = 0;
    Confirmacion rivalDelante;
};

#endif
