#include "movimiento/RutinaInicio.h"
#include "ParametrosModo4.h"

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
    const int tactica = tacticaDeModo(rutina);
    if (tactica != 0) {
        if (!tacticaRapida(tactica, decision, mando, motor, ahora, t, ataque)) {
            rutina = 0;
            return false;
        }
        return true;
    }
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
        sigue = avanzarAlCentro(decision, mando, motor, ahora, t, ataque);
    } else if (rutina == 4) {
        sigue = explorar(decision, mando, motor, ahora, t, ataque);
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

// Ronda 3: avanzar despacio hacia delante. Al ver al rival por delante va contra
// él acelerando poco a poco desde esta velocidad lenta; si lo ve de lado,
// termina y el control normal gira hacia él.
bool RutinaInicio::avanzarAlCentro(const DecisionMovimiento& decision, MandoMotores& mando,
                                   IMotor& motor, unsigned long ahora, unsigned long t,
                                   ControlAtaque& ataque) {
    const bool esperando = EsperarRound3 && t >= TiempoAvanceInicio;
    const bool visto = decision.tipo != TipoAccion::Busqueda;
    const bool confirmado = rivalVisto.actualizar(visto, ahora);
    const unsigned long fin = TiempoAvanceInicio + (EsperarRound3 ? TiempoEsperaRound3 : 0);
    // Una detección no termina el avance antes de TiempoMinimoAvanceInicio
    const bool encontrado = confirmado && t >= TiempoMinimoAvanceInicio;
    const bool sigue = !encontrado && t < fin;
    if (encontrado && esAtaque(decision.tipo)) {
        ataque.iniciarAproximacion(ahora);
    }
    if (sigue && esperando) {
        mando.mover(motor, 0, 0, true);
    } else if (sigue) {
        mando.mover(motor, VelocidadAvanceInicio, VelocidadAvanceInicio);
    }
    return sigue;
}

// Ronda 4 (salida libre, en cualquier parte del dohyo y con cualquier rumbo):
// girar en el sitio buscando al rival. Cualquier sensor que lo vea termina la
// rutina: el control normal gira hacia él (laterales) o lo ataca (frontales).
bool RutinaInicio::explorar(const DecisionMovimiento& decision, MandoMotores& mando,
                            IMotor& motor, unsigned long ahora, unsigned long t, ControlAtaque& ataque) {
    const bool confirmado = rivalVisto.actualizar(decision.tipo != TipoAccion::Busqueda, ahora);
    const bool sigue = !confirmado && t < TiempoMaxGiroRound4;
    if (sigue) {
        mando.mover(motor, lado * VelocidadGiroRound4, -lado * VelocidadGiroRound4, true);
    }
    return sigue;
}

// Tácticas rápidas de Rumania (ver TacticaModo1..4 en ParametrosModo4.h). En
// cualquier fase, el sensor frontal lo ve (confirmado): ataque a fondo. Se mira
// el sensor y no la decisión: la red puede proponer girar aunque no vea nada.
bool RutinaInicio::tacticaRapida(int tactica, const DecisionMovimiento& decision, MandoMotores& mando,
                                 IMotor& motor, unsigned long ahora, unsigned long t, ControlAtaque& ataque) {
    if (rivalVisto.actualizar(decision.enemigoFrente, ahora)) {
        // Espera y esquiva: si lo ve pronto, viene embistiendo; primero se aparta
        if (tactica == TacticaEsperaEsquiva && t < VentanaRivalRapidoTactica && !esquivado) {
            esquivando = true;
            esquivado = true;   // una sola esquiva por combate
            inicioEsquiva = ahora;
        } else if (!esquivando) {
            ataque.iniciarAtaqueMaximo(ahora);
            return false;
        }
    }
    const bool espera = tactica == TacticaContragolpe || tactica == TacticaEsperaEsquiva;
    if (t >= (espera ? TiempoMaxEsperaTactica : TiempoMaxTactica)) {
        return false;
    }
    // Lado en el que lo ve un sensor de 45° o lateral (0 = ninguno). AjusteIzq/Der,
    // CorregirIzq/Der y DefensaIzq/Der van seguidos en TipoAccion, alternando lados
    const int k = (int)decision.tipo - (int)TipoAccion::AjusteIzq;
    const int visto = k >= 0 && k <= (int)TipoAccion::DefensaDer - (int)TipoAccion::AjusteIzq ? (k & 1 ? 1 : -1) : 0;
    const int v = VelocidadTactica;
    if (esquivando) {
        // Se aparta pivotando hacia DIP3 y después gira hacia el otro lado para pillarlo de lado
        if (ahora - inicioEsquiva < TiempoEsquivaTactica) {
            mando.mover(motor, lado > 0 ? v : 0, lado > 0 ? 0 : v, true);
            return true;
        }
        if (ahora - inicioEsquiva < 2 * TiempoEsquivaTactica) {
            mando.mover(motor, -lado * VelocidadGiroTactica, lado * VelocidadGiroTactica, true);
            return true;
        }
        esquivando = false;   // sigue con el giro hacia el sensor que lo vea
    }
    int sentido = lado;
    if (tactica == TacticaCarga && visto == 0) {
        mando.mover(motor, v, v, true);                           // carga recta a fondo
        return true;
    }
    if (espera && visto == 0) {
        mando.mover(motor, 0, 0, true);                           // quieto hasta verlo
        return true;
    }
    if (tactica == TacticaPivote && visto == 0) {
        mando.mover(motor, lado > 0 ? v : 0, lado > 0 ? 0 : v, true);  // pivote hacia DIP3
        return true;
    }
    if (tactica == TacticaArco) {
        if (t < TiempoArcoLibre) {
            // 斜め / 回り込み: arco a fondo abriéndose hacia DIP3
            mando.mover(motor, lado > 0 ? v : VelocidadInteriorArcoLibre, lado > 0 ? VelocidadInteriorArcoLibre : v, true);
            return true;
        }
        sentido = -lado;                                          // y después hacia dentro
    }
    if (tactica == TacticaHenka && t < TiempoRetrocesoLibre) {
        // 受け: la rueda del lado de DIP3 atrás a fondo; sale de la línea de embestida
        mando.mover(motor, lado > 0 ? 0 : -v, lado > 0 ? -v : 0, true);
        return true;
    }
    // Giro rápido en el sitio hacia el sensor que lo ve (o hacia DIP3)
    if (visto != 0) sentido = visto;
    mando.mover(motor, sentido * VelocidadGiroTactica, -sentido * VelocidadGiroTactica, true);
    return true;
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
