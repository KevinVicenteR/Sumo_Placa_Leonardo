#include "maniobras/GiroLateral.h"

void GiroLateral::girarHacia(MandoMotores& mando, IMotor& motor, int lado) {
    if (GiroLateralEnRueda) {
        mando.mover(motor, lado > 0 ? VelocidadRuedaPivote : 0, lado > 0 ? 0 : VelocidadRuedaPivote);
    } else {
        mando.mover(motor, lado * VelocidadPivoteLateral, -lado * VelocidadPivoteLateral);
    }
}

bool GiroLateral::continuar(const DecisionMovimiento& decision, MandoMotores& mando, IMotor& motor,
                            unsigned long ahora) {
    if (esDefensa(decision.tipo)) {
        const int lado = decision.tipo == TipoAccion::DefensaIzq ? -1 : 1;
        if (sentido != lado) {
            sentido = lado;
            inicio = ahora;
            rivalDelante.reiniciar();
        }
    }
    if (sentido == 0) {
        return false;
    }

    const bool otroLado = (sentido < 0 && decision.tipo == TipoAccion::CorregirDer) ||
                          (sentido > 0 && decision.tipo == TipoAccion::CorregirIzq);
    const bool confirmado = rivalDelante.actualizar(veDeFrente(decision) || otroLado, ahora);
    if ((confirmado && ahora - inicio >= TiempoMinimoGiroLateral) ||
        ahora - inicio >= TiempoMaxGiroLateral) {
        sentido = 0;
        return false;
    }
    girarHacia(mando, motor, sentido);
    return true;
}
