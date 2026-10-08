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
    // Ronda 2: el lado del giro lo decide el primer sensor lateral o de 45° que
    // vea al rival, también durante la esquiva (al avanzar, el rival queda atrás
    // y el sensor lateral deja de verlo)
    if (rutina == 2) {
        decidirLado(decision, busqueda);
    }
    // Rondas 1 y 2: primero se aparta de la embestida avanzando recto
    const unsigned long esquiva = rutina == 1 ? TiempoEsquivaRound1 : rutina == 2 ? TiempoEsquivaRound2 : 0;
    if (t < esquiva) {
        mando.mover(motor, VelocidadEsquiva, VelocidadEsquiva, true);
        return true;
    }
    if (rutina == 1 || rutina == 2) {
        // El giro cuenta su tiempo desde que termina la esquiva
        const unsigned long tGiro = t - esquiva;
        sigue = girar(decision, mando, motor, ahora, tGiro, busqueda, ataque);
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
    // Una detección no termina el giro antes de su mínimo ni sin confirmarse
    const bool confirmado = rivalVisto.actualizar(veDeFrente(decision), ahora);
    const bool encontrado = confirmado && t >= minimo;
    const bool sigue = !encontrado && t < maximo;


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
        // Giro en el sitio (ronda 1: media vuelta hacia el lado de DIP3)
        const int velocidad = rutina == 2 ? VelocidadGiroRound2 : VelocidadGiroInicio;
        mando.mover(motor, lado * velocidad, -lado * velocidad, true);
    }
    return sigue;
}

// Ronda 3: avanzar hacia el centro y después esperar quieto al rival
bool RutinaInicio::avanzarAlCentro(const DecisionMovimiento& decision, MandoMotores& mando,
                                   IMotor& motor, unsigned long ahora, unsigned long t) {
    const bool esperando = EsperarRound3 && t >= TiempoAvanceInicio;
    // Cualquier sensor (frontal, 45° o lateral) que vea al rival termina la
    // rutina: el control normal gira hacia él y ataca
    const bool visto = decision.tipo != TipoAccion::Busqueda;
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

// Ronda 2: un sensor lateral o de 45° que ve al rival fija el lado del giro (si
// ninguno lo ve, se usa el lado de DIP3). Una vez decidido, no cambia.
void RutinaInicio::decidirLado(const DecisionMovimiento& decision, BusquedaRival& busqueda) {
    if (ladoDecidido) {
        return;
    }
    const TipoAccion t = decision.tipo;
    const bool izquierda = t == TipoAccion::DefensaIzq || t == TipoAccion::CorregirIzq;
    const bool derecha = t == TipoAccion::DefensaDer || t == TipoAccion::CorregirDer;
    if (!izquierda && !derecha) {
        return;
    }
    lado = izquierda ? -1 : 1;
    busqueda.fijarSentido(lado);
    ladoDecidido = true;
}
