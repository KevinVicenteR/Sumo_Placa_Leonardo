#include "maniobras/ParoPerdida.h"

bool ParoPerdida::continuar(const DecisionMovimiento& decision, MandoMotores& mando, IMotor& motor,
                            unsigned long ahora) {
    if (decision.tipo == TipoAccion::Busqueda && veniaAtacando &&
        mando.calcularFreno(TiempoParoPerdida, freno)) {
        frenando = true;
        inicio = ahora;
    }
    veniaAtacando = esAtaque(decision.tipo);
    if (!frenando) {
        return false;
    }
    if (decision.tipo != TipoAccion::Busqueda || ahora - inicio >= freno.duracion) {
        frenando = false;
        return false;
    }
    mando.mover(motor, freno.izq, freno.der, true, true);
    return true;
}
