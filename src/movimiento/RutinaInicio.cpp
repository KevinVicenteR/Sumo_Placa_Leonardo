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
        fase = 0;
        viendoLateral = false;
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
        sigue = avanzarAlCentro(decision, mando, motor, ahora, t, ataque);
    } else if (rutina == 4) {
        sigue = explorar(decision, mando, motor, ahora, t);
    } else if (rutina >= 5 && rutina <= 8) {
        sigue = esperarRival(decision, mando, motor, ahora, t, ataque);
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
                            IMotor& motor, unsigned long ahora, unsigned long t) {
    const bool confirmado = rivalVisto.actualizar(decision.tipo != TipoAccion::Busqueda, ahora);
    const bool sigue = !confirmado && t < TiempoMaxGiroRound4;
    if (sigue) {
        mando.mover(motor, lado * VelocidadGiroRound4, -lado * VelocidadGiroRound4, true);
    }
    return sigue;
}

// --- Estrategias de espera (rutinas 5 a 8, colocación libre) ---

static bool variosFrontales(const DecisionMovimiento& d) {
    return d.ataqueDirecto || d.tipo == TipoAccion::AjusteIzq || d.tipo == TipoAccion::AjusteDer;
}

// -1 / 1 si lo ve el sensor lateral izquierdo / derecho; 0 si no
static int ladoLateral(TipoAccion t) {
    return t == TipoAccion::DefensaIzq ? -1 : t == TipoAccion::DefensaDer ? 1 : 0;
}

// -1 / 1 si lo ve solo el sensor de 45° izquierdo / derecho; 0 si no
static int lado45(TipoAccion t) {
    return t == TipoAccion::CorregirIzq ? -1 : t == TipoAccion::CorregirDer ? 1 : 0;
}

void RutinaInicio::empezarFase(int nueva, unsigned long ahora) {
    fase = nueva;
    inicioFase = ahora;
    rivalVisto.reiniciar();
}

bool RutinaInicio::pasarA(int nueva, unsigned long ahora, MandoMotores& mando, IMotor& motor) {
    rutina = nueva;
    inicio = ahora;
    empezarFase(0, ahora);
    mando.mover(motor, 0, 0, true);
    return true;
}

bool RutinaInicio::esperarRival(const DecisionMovimiento& decision, MandoMotores& mando,
                                IMotor& motor, unsigned long ahora, unsigned long t,
                                ControlAtaque& ataque) {
    if (fase == 1) {
        return pivotarHaciaRival(decision, mando, motor, ahora, ataque);
    }
    if (fase == 2) {
        // Rutina 8: se aparta del camino del rival pivotando hacia DIP3; después
        // el control normal lo busca (pasará de largo y lo verá un lateral)
        if (ahora - inicioFase >= TiempoEsquivaLibre) {
            return false;
        }
        mando.limites.golpe = true;
        mando.mover(motor, lado > 0 ? VelocidadEsquivaLibre : 0, lado > 0 ? 0 : VelocidadEsquivaLibre, true);
        return true;
    }

    const TipoAccion tipo = decision.tipo;
    const bool confirmado = rivalVisto.actualizar(tipo != TipoAccion::Busqueda, ahora);

    // Sin ver nada durante TiempoMaxEsperaRival: sale a buscarlo
    if (t >= TiempoMaxEsperaRival && tipo == TipoAccion::Busqueda) {
        if (rutina == 6 || rutina == 7) {
            ladoGolpe = lado;                        // pivota hacia DIP3 (hacia el rival)
            empezarFase(1, ahora);
            return pivotarHaciaRival(decision, mando, motor, ahora, ataque);
        }
        return pasarA(3, ahora, mando, motor);       // avanza despacio (ronda 3)
    }

    if (rutina == 8) {
        // Si el frontal lo ve antes de VentanaRivalRapido desde la salida (ya se
        // ha acercado), viene embistiendo rápido: se aparta. Si no, golpe
        // cuando está cerca (dos o tres frontales).
        const bool rapido = t <= VentanaRivalRapido;
        if (confirmado && (rapido ? veDeFrente(decision) : variosFrontales(decision))) {
            if (rapido) {
                empezarFase(2, ahora);
                mando.limites.golpe = true;
        mando.mover(motor, lado > 0 ? VelocidadEsquivaLibre : 0, lado > 0 ? 0 : VelocidadEsquivaLibre, true);
                return true;
            }
            ataque.iniciarGolpe(ahora);
            return false;
        }
    } else if (rutina == 5) {
        // Contragolpe: golpe a fondo cuando dos o tres frontales lo ven (cerca),
        // o ya al verlo el frontal con ContragolpeAlVerlo
        const bool listo = ContragolpeAlVerlo ? veDeFrente(decision) : variosFrontales(decision);
        if (listo && confirmado) {
            ataque.iniciarGolpe(ahora);
            return false;
        }
    } else {
        // Rutinas 6 y 7: de frente, golpe directo
        if (veDeFrente(decision) && confirmado) {
            ataque.iniciarGolpe(ahora);
            return false;
        }
    }

    const int lat = ladoLateral(tipo);
    const int diag = lado45(tipo);
    if (rutina == 6 && confirmado && (lat != 0 || diag != 0)) {
        // Emboscada: el rival pasa por el lateral o el 45°, pivota y lo golpea de lado
        ladoGolpe = lat != 0 ? lat : diag;
        empezarFase(1, ahora);
        return pivotarHaciaRival(decision, mando, motor, ahora, ataque);
    }
    if (rutina == 7) {
        // De lado: espera mientras solo lo ve el lateral; si sigue ahí
        // TiempoLateralCerca o pasa al 45°, pivota y lo golpea
        if (lat == 0) {
            viendoLateral = false;
        } else if (!viendoLateral) {
            viendoLateral = true;
            inicioLateral = ahora;
        }
        const bool encima = viendoLateral && ahora - inicioLateral >= TiempoLateralCerca;
        if ((diag != 0 && confirmado) || encima) {
            ladoGolpe = diag != 0 ? diag : lat;
            empezarFase(1, ahora);
            return pivotarHaciaRival(decision, mando, motor, ahora, ataque);
        }
        mando.mover(motor, 0, 0, true);
        return true;
    }
    if ((rutina == 5 || rutina == 8) && lat != 0 && confirmado) {
        // Lo ve de lado: el control normal gira hacia él
        return false;
    }
    if ((rutina == 5 || rutina == 8) && diag != 0) {
        // Solo un 45°: gira en el sitio para centrarlo, sin avanzar
        mando.mover(motor, diag * VelocidadGiroEspera, -diag * VelocidadGiroEspera, true);
        return true;
    }
    // Nada, o solo el frontal central (aún lejos): quieto (o, con
    // ContragolpeAvanzaLento, avanzando despacio hacia él)
    if ((rutina == 5 || rutina == 8) && ContragolpeAvanzaLento && veDeFrente(decision)) {
        mando.mover(motor, VelocidadAvanceInicio, VelocidadAvanceInicio);
        return true;
    }
    mando.mover(motor, 0, 0, true);
    return true;
}

// Pivota sobre la rueda del lado del rival hasta tenerlo de frente y lo golpea
bool RutinaInicio::pivotarHaciaRival(const DecisionMovimiento& decision, MandoMotores& mando,
                                     IMotor& motor, unsigned long ahora, ControlAtaque& ataque) {
    if (rivalVisto.actualizar(veDeFrente(decision), ahora)) {
        ataque.iniciarGolpe(ahora);
        return false;
    }
    if (ahora - inicioFase >= TiempoMaxGiroLado) {
        return false;
    }
    mando.limites.golpe = true;
    const int velocidad = rutina == 7 ? VelocidadPivoteDeLado : VelocidadPivoteEmboscada;
    mando.mover(motor, ladoGolpe > 0 ? velocidad : 0, ladoGolpe > 0 ? 0 : velocidad, true);
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
