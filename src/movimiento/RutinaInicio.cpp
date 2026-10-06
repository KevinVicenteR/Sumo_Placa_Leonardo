#include "movimiento/RutinaInicio.h"

void RutinaInicio::iniciar(int rutinaElegida, int ladoElegido) {
    rutina = rutinaElegida;
    lado = ladoElegido < 0 ? -1 : 1;
    enCurso = false;
    conRutina = rutinaElegida != 0;
}

bool RutinaInicio::continuar(const DecisionMovimiento& decision, MandoMotores& mando, IMotor& motor,
                             unsigned long ahora, BusquedaRival& busqueda, ControlAtaque& ataque) {
    if (rutina == 0) {
        return false;
    }
    // Primer ciclo de la rutina
    if (!enCurso) {
        enCurso = true;
        inicio = ahora;
        rivalVisto.reiniciar();
        ladoDecidido = false;
        busqueda.fijarSentido(lado);
    }

    // Frenando el giro tras encontrar al rival
    if (frenandoGiro) {
        const bool sigue = ahora - inicioFreno < freno.duracion;
        if (sigue) {
            mando.mover(motor, freno.izq, freno.der, true);
        } else {
            frenandoGiro = false;
            rutina = 0;
        }
        return sigue;
    }

    const unsigned long t = ahora - inicio;
    bool sigue = false;
    if (rutina == 1 || rutina == 2) {
        sigue = girar(decision, mando, motor, ahora, t, busqueda, ataque);
    } else if (rutina == 3) {
        sigue = avanzarAlCentro(decision, mando, motor, ahora, t);
    }
    if (!sigue) {
        rutina = 0;
    }
    return sigue;
}

// Rondas 1 y 2: girar hasta ver al rival de frente (o agotar el tiempo)
bool RutinaInicio::girar(const DecisionMovimiento& decision, MandoMotores& mando, IMotor& motor,
                         unsigned long ahora, unsigned long t, BusquedaRival& busqueda,
                         ControlAtaque& ataque) {
    const unsigned long maximo = rutina == 1 ? TiempoMaxGiroEspalda : TiempoMaxGiroLado;
    const unsigned long minimo = rutina == 1 ? TiempoMinimoGiroEspalda : TiempoMinimoGiroLado;
    const bool confirmado = rivalVisto.actualizar(veDeFrente(decision), ahora);
    const bool encontrado = confirmado && t >= minimo;
    const bool sigue = !encontrado && t < maximo;

    // Ronda 2: si un sensor lateral ve al rival, el giro va hacia ese lado
    if (rutina == 2 && !ladoDecidido && esDefensa(decision.tipo)) {
        lado = decision.tipo == TipoAccion::DefensaIzq ? -1 : 1;
        busqueda.fijarSentido(lado);
        ladoDecidido = true;
    }

    // Rival encontrado: embestida, frenando antes el giro si va rápido
    if (encontrado) {
        ataque.iniciarEmbestida(ahora);
        if (mando.calcularFreno(TiempoFrenadoRuedas, freno)) {
            frenandoGiro = true;
            inicioFreno = ahora;
            mando.mover(motor, freno.izq, freno.der, true);
            return true;
        }
    }

    if (sigue && rutina == 2 && PivoteRound2) {
        // Pivote sobre la rueda del lado del rival
        mando.mover(motor, lado > 0 ? VelocidadPivoteInicio : 0, lado > 0 ? 0 : VelocidadPivoteInicio, true);
    } else if (sigue) {
        // Giro en el sitio
        mando.mover(motor, lado * VelocidadGiroInicio, -lado * VelocidadGiroInicio, true);
    }
    return sigue;
}

// Ronda 3: avanzar hacia el centro y después esperar quieto al rival
bool RutinaInicio::avanzarAlCentro(const DecisionMovimiento& decision, MandoMotores& mando,
                                   IMotor& motor, unsigned long ahora, unsigned long t) {
    const bool porDelante = veDeFrente(decision) || decision.tipo == TipoAccion::CorregirIzq ||
                            decision.tipo == TipoAccion::CorregirDer;
    const bool esperando = EsperarRound3 && t >= TiempoAvanceInicio;
    // Avanzando reacciona a lo que ve delante; esperando, a cualquier sensor
    const bool visto = esperando ? decision.tipo != TipoAccion::Busqueda : porDelante;
    const bool confirmado = rivalVisto.actualizar(visto, ahora);
    const unsigned long fin = TiempoAvanceInicio + (EsperarRound3 ? TiempoEsperaRound3 : 0);
    const bool sigue = !(confirmado && t >= TiempoMinimoAvanceInicio) && t < fin;
    if (sigue && esperando) {
        mando.mover(motor, 0, 0, true);
    } else if (sigue) {
        mando.mover(motor, VelocidadAvanceInicio, VelocidadAvanceInicio);
    }
    return sigue;
}
