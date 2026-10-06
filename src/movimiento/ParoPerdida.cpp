#include "movimiento/ParoPerdida.h"

bool ParoPerdida::continuar(const DecisionMovimiento& decision, MandoMotores& mando, IMotor& motor,
                            unsigned long ahora) {
    // Venía atacando y ahora no ve nada: empezar a frenar (si va rápido)
    if (decision.tipo == TipoAccion::Busqueda && veniaAtacando &&
        mando.calcularFreno(TiempoParoPerdida, freno)) {
        frenando = true;
        inicio = ahora;
    }
    veniaAtacando = esAtaque(decision.tipo);
    if (!frenando) {
        return false;
    }
    // Termina si vuelve a ver algo o se acaba el tiempo de frenado
    if (decision.tipo != TipoAccion::Busqueda || ahora - inicio >= freno.duracion) {
        frenando = false;
        return false;
    }
    mando.mover(motor, freno.izq, freno.der, true);
    return true;
}
