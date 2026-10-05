#include "ControlMovimiento.H"
#include "Parametros.H"

namespace {
long magnitud(long v) { return v < 0 ? -v : v; }

bool esBorde(TipoAccion tipo) {
    return tipo == TipoAccion::EvadirBordeIzq || tipo == TipoAccion::EvadirBordeDer ||
           tipo == TipoAccion::EvadirBordeAmbos;
}

// El sensor frontal ve al enemigo (las decisiones de ataque frontal lo implican)
bool veDeFrente(const DecisionMovimiento& decision) {
    return decision.enemigoFrente || decision.tipo == TipoAccion::AtaqueFrontal ||
           decision.tipo == TipoAccion::AjusteIzq || decision.tipo == TipoAccion::AjusteDer;
}

bool esAtaque(TipoAccion tipo) {
    return tipo == TipoAccion::AtaqueFrontal || tipo == TipoAccion::AjusteIzq ||
           tipo == TipoAccion::AjusteDer || tipo == TipoAccion::CorregirIzq ||
           tipo == TipoAccion::CorregirDer;
}
}

// Sube la orden como mucho pasoRampa; bajarla o pararla es inmediato, y al
// cambiar de sentido pasa por cero y vuelve a subir con la rampa
int ControlMovimiento::conRampa(int actual, int objetivo) const {
    const long producto = (long)actual * objetivo;
    if (objetivo == 0 || (producto > 0 && magnitud(objetivo) <= magnitud(actual))) {
        return objetivo;
    }
    const int base = producto > 0 ? actual : 0;
    const int diferencia = objetivo - base;
    if (diferencia > pasoRampa) return base + pasoRampa;
    if (diferencia < -pasoRampa) return base - pasoRampa;
    return objetivo;
}

void ControlMovimiento::mover(IMotor& motor, int izq, int der, bool urgente) {
    // Hacia delante (incluido pivotar sobre una rueda) se limita la velocidad:
    // cerca del borde visto y según el borde previsto por el estimador
    const int mayor = izq > der ? izq : der;
    if (izq >= 0 && der >= 0 && mayor > 0) {
        int limite = limiteArranque;
        if (limiteSalidaSuave < limite) limite = limiteSalidaSuave;
        if (limitarAvance && VelocidadCercaBorde < limite) limite = VelocidadCercaBorde;
        if (UsarEstimadorBorde && (!empujando || EstimadorLimitaEmbestida) && limiteEstimador < limite) {
            limite = limiteEstimador;
        }
        if (mayor > limite) {
            izq = (long)izq * limite / mayor;
            der = (long)der * limite / mayor;
        }
    }
    if (rampa > 0 && !urgente) {
        izq = conRampa(ordenIzq, izq);
        der = conRampa(ordenDer, der);
    }
    ordenIzq = izq;
    ordenDer = der;
    motor.mover(izq, der);
}

// Velocidad estimada de cada rueda (en unidades de PWM): sigue a la orden con la
// inercia del robot, como un filtro de primer orden con constante TauRuedas
void ControlMovimiento::estimarVelocidad(unsigned long ahora) {
    const unsigned long dt = ahora - ultimaEstimacion;
    if (dt == 0) {
        return;
    }
    ultimaEstimacion = ahora;
    const long paso = dt >= TauRuedas ? 256 : (long)dt * 256 / TauRuedas;
    estIzq += ((long)ordenIzq * 256 - estIzq) * paso / 256;
    estDer += ((long)ordenDer * 256 - estDer) * paso / 256;
}

// Freno de cada rueda en proporción a su velocidad estimada: la más rápida a
// tope en sentido contrario, durante más tiempo cuanto más rápido iba.
// Devuelve false si el robot apenas se movía.
bool ControlMovimiento::calcularFreno(unsigned long duracionMaxima) {
    const long izq = estIzq / 256, der = estDer / 256;
    const long mayor = magnitud(izq) > magnitud(der) ? magnitud(izq) : magnitud(der);
    if (mayor < VelocidadMinimaFreno) {
        return false;
    }
    frenoIzq = -izq * VelocidadFreno / mayor;
    frenoDer = -der * VelocidadFreno / mayor;
    duracionFreno = duracionMaxima * mayor / VelocidadMaxima;
    return true;
}

void ControlMovimiento::iniciarEvasion(int sentido, unsigned long duracionRet,
                                       unsigned long duracionGir, unsigned long ahora, bool unSensor) {
    // No reiniciar el presupuesto de recuperación por lecturas repetidas.
    if (fase != Fase::Libre) {
        // Si aparece el segundo sensor, recuperar recto sin reiniciar el límite.
        if (evasionUnSensor && !unSensor) {
            evasionUnSensor = false;
            duracionGiro = duracionGir;
            negroContinuo = false;
        }
        return;
    }
    evasionUnSensor = unSensor;
    salidaSuaveUnSensor = false;
    limiteSalidaSuave = 255;
    fase = Fase::Retrocediendo;
    inicioFase = ahora;
    inicioEvasion = ahora;
    negroContinuo = false;
    duracionRetroceso = duracionRet;
    duracionGiro = duracionGir;
    sentidoGiro = sentido;
    enParo = false;
    veniaAtacando = false;
    buscando = false;
    evasionCompleta = false;
    giroLateral = 0;
    viendoFrente = false;
    empujando = false;
}

bool ControlMovimiento::continuarEvasion(const DecisionMovimiento& decision, IMotor& motor, unsigned long ahora) {
    // Recuperar también si la pausa o el giro vuelven a dejar un sensor en blanco.
    if (fase != Fase::Libre && fase != Fase::Retrocediendo && esBorde(decision.tipo) &&
        ahora - inicioEvasion < TiempoMaximoRecuperacionBorde) {
        fase = Fase::Retrocediendo;
        inicioFase = ahora;
        duracionRetroceso = 0;
        negroContinuo = false;
    }
    if (fase == Fase::Retrocediendo) {
        if (esBorde(decision.tipo)) {
            negroContinuo = false;
        } else if (!negroContinuo) {
            negroContinuo = true;
            inicioNegro = ahora;
        }
        const unsigned long separacion = evasionUnSensor ? TiempoSeparacionUnSensor : TiempoSeparacionBorde;
        const bool separado = negroContinuo && ahora - inicioNegro >= separacion &&
                              ahora - inicioFase >= duracionRetroceso;
        const bool agotado = ahora - inicioEvasion >= TiempoMaximoRecuperacionBorde;
        if (!separado && !agotado) {
            const int velocidad = evasionUnSensor ? VelocidadRetrocesoUnSensor : VelocidadRetroceso;
            mover(motor, -velocidad, -velocidad, true);
            return true;
        }
        fase = Fase::Frenando;
        inicioFase = ahora;
    }

    if (fase == Fase::Frenando) {
        // No pivotar con un sensor aún fuera. Tampoco retroceder sin límite:
        // el robot no tiene sensores traseros.
        if (esBorde(decision.tipo) || ahora - inicioFase < TiempoFrenado) {
            mover(motor, 0, 0, true);
            return true;
        }
        fase = Fase::Girando;
        inicioFase = ahora;
    }

    if (fase == Fase::Girando) {
        // Si el giro vuelve a tocar blanco, pararlo inmediatamente.
        if (!esBorde(decision.tipo) && ahora - inicioFase < duracionGiro) {
            mover(motor, sentidoGiro * VelocidadGiroEvasion,
                  -sentidoGiro * VelocidadGiroEvasion, true);
            return true;
        }
        fase = Fase::Asentando;
        inicioFase = ahora;
    }

    if (fase == Fase::Asentando) {
        // No volver a avanzar mientras las ruedas aún conservan el giro.
        if (esBorde(decision.tipo) || ahora - inicioFase < TiempoAsentamientoEvasion) {
            mover(motor, 0, 0, true);
            return true;
        }
        // La búsqueda debe continuar hacia el interior, no hacia el último
        // enemigo que pudo haberse visto fuera del borde.
        if (evasionUnSensor) {
            sentidoBusqueda = sentidoGiro;
            salidaSuaveUnSensor = true;
            inicioSalidaSuave = ahora;
            limiteSalidaSuave = VelocidadSalidaUnSensor;
        }
        evasionCompleta = decision.tipo == TipoAccion::Busqueda;
        fase = Fase::Libre;
    }

    return false;
}

void ControlMovimiento::recordarLadoEnemigo(TipoAccion tipo) {
    switch (tipo) {
    case TipoAccion::AjusteIzq:
    case TipoAccion::CorregirIzq:
    case TipoAccion::DefensaIzq:
        sentidoBusqueda = -1;
        break;
    case TipoAccion::AjusteDer:
    case TipoAccion::CorregirDer:
    case TipoAccion::DefensaDer:
        sentidoBusqueda = 1;
        break;
    default:
        break;
    }
}

// Búsqueda en arcos suaves: avanza sin pararse curvando hacia donde se vio al
// enemigo por última vez y cambia el lado de la curva cada TiempoArcoBusqueda
void ControlMovimiento::buscarEnArcos(IMotor& motor, unsigned long ahora) {
    if (!buscando) {
        buscando = true;
        inicioBusqueda = ahora;
        evasionCompleta = false;
    }
    const unsigned long transcurrido = ahora - inicioBusqueda;
    const unsigned long tramo = transcurrido / TiempoArcoBusqueda;
    const unsigned long progreso = transcurrido % TiempoArcoBusqueda;
    const int interior = VelocidadAvance * PorcentajeArcoBusqueda / 100;
    // Interpolar entre curvas: no intercambiar de golpe las velocidades.
    const int cambio = (long)(VelocidadAvance - interior) * progreso / TiempoArcoBusqueda;
    int izq = tramo % 2 == 0 ? VelocidadAvance - cambio : interior + cambio;
    int der = tramo % 2 == 0 ? interior + cambio : VelocidadAvance - cambio;
    if (sentidoBusqueda < 0) {
        const int temporal = izq;
        izq = der;
        der = temporal;
    }
    // Sin pulso inicial: subir desde cero al entrar en búsqueda.
    if (TiempoArranqueBusqueda > 0 && transcurrido < TiempoArranqueBusqueda) {
        izq = (long)izq * transcurrido / TiempoArranqueBusqueda;
        der = (long)der * transcurrido / TiempoArranqueBusqueda;
    }
    mover(motor, izq, der);
}

void ControlMovimiento::buscar(IMotor& motor, unsigned long ahora) {
    if (patron == 1) {
        buscarEnArcos(motor, ahora);
        return;
    }
    if (!buscando) {
        buscando = true;
        // Solo se empieza avanzando si una evasión completa dejó al robot mirando
        // hacia el centro; si no, podría estar de frente al borde y se gira primero
        inicioBusqueda = evasionCompleta ? ahora : ahora - TiempoAvanceBusqueda - TiempoPausaBusqueda;
        evasionCompleta = false;
    }
    // Ciclo: avance corto -> pausa -> giro en el sitio -> pausa. Las pausas frenan
    // el robot entre un movimiento y otro: al girar en el sitio las ruedas patinan
    // en sentidos opuestos y no frenan el avance que traía, así que sin la primera
    // el robot se desliza hacia delante durante todo el giro y puede acabar de
    // lado sobre la línea, donde los sensores de piso no la ven. Sin la segunda
    // empieza a avanzar aún girando por la inercia y sale en curva.
    const unsigned long finAvance = TiempoAvanceBusqueda;
    const unsigned long inicioGiro = finAvance + TiempoPausaBusqueda;
    const unsigned long finGiro = inicioGiro + TiempoGiroBusqueda;
    const unsigned long t = (ahora - inicioBusqueda) % (finGiro + TiempoPausaBusqueda);
    if (t < finAvance) {
        avanzarPorPulsos(motor, t);
    } else if (t >= inicioGiro && t < finGiro) {
        mover(motor, sentidoBusqueda * VelocidadGiroBusqueda, -sentidoBusqueda * VelocidadGiroBusqueda);
    } else {
        mover(motor, 0, 0);
    }
}

// Avance de búsqueda en pulsos: empuja TiempoPulsoAvance y frena
// TiempoPausaPulso, así el robot nunca llega a coger velocidad y, si un sensor
// ve la línea, para casi en el sitio. Con TiempoPulsoAvance = 0 avanza seguido.
void ControlMovimiento::avanzarPorPulsos(IMotor& motor, unsigned long t) {
    if (TiempoPulsoAvance > 0 && t % (TiempoPulsoAvance + TiempoPausaPulso) >= TiempoPulsoAvance) {
        mover(motor, 0, 0);
        return;
    }
    // Zigzag: arcos alternos hacia uno y otro lado, para barrer con los sensores
    // delanteros sin parar a girar (PorcentajeArcoZigzag = 100 avanza recto)
    const int interior = VelocidadAvance * PorcentajeArcoZigzag / 100;
    const bool haciaIzq = TiempoArcoZigzag > 0 && (t / TiempoArcoZigzag) % 2 == 0;
    if (haciaIzq) {
        mover(motor, interior, VelocidadAvance);
    } else {
        mover(motor, VelocidadAvance, interior);
    }
}

// Enemigo de lado: gira hacia él frenando la rueda de ese lado, que hace de
// pivote, mientras la otra empuja (o girando en el sitio, según GiroLateralEnRueda)
void ControlMovimiento::girarHaciaLado(IMotor& motor, int sentido) {
    if (GiroLateralEnRueda) {
        mover(motor, sentido > 0 ? VelocidadRuedaPivote : 0, sentido > 0 ? 0 : VelocidadRuedaPivote);
    } else {
        mover(motor, sentido * VelocidadPivoteLateral, -sentido * VelocidadPivoteLateral);
    }
}

// Un sensor lateral vio al enemigo: el giro sigue hasta que lo vea el sensor
// frontal, aunque al girar pase por el hueco entre el lateral y el de 45°
bool ControlMovimiento::continuarGiroLateral(const DecisionMovimiento& decision, IMotor& motor, unsigned long ahora) {
    if (decision.tipo == TipoAccion::DefensaIzq || decision.tipo == TipoAccion::DefensaDer) {
        const int sentido = decision.tipo == TipoAccion::DefensaIzq ? -1 : 1;
        if (giroLateral != sentido) {
            giroLateral = sentido;
            inicioGiroLateral = ahora;
            detectaGiroLateral = false;
        }
    }
    if (giroLateral == 0) {
        return false;
    }
    const bool otroLado = (giroLateral < 0 && decision.tipo == TipoAccion::CorregirDer) ||
                          (giroLateral > 0 && decision.tipo == TipoAccion::CorregirIzq);
    // Termina al tener al enemigo de frente (o pasado al otro lado), confirmado y
    // tras el giro mínimo; una lectura suelta no lo corta
    const bool confirmado = confirmar(veDeFrente(decision) || otroLado, ahora, detectaGiroLateral,
                                      inicioDetectaGiroLateral);
    if ((confirmado && ahora - inicioGiroLateral >= TiempoMinimoGiroLateral) ||
        ahora - inicioGiroLateral >= TiempoMaxGiroLateral) {
        giroLateral = 0;
        return false;
    }
    girarHaciaLado(motor, giroLateral);
    return true;
}

// Si iba atacando y el enemigo desaparece (lo esquivó), frena en seco en vez de
// seguir lanzado: a toda velocidad el robot no alcanzaría a parar al ver la línea
bool ControlMovimiento::continuarParo(const DecisionMovimiento& decision, IMotor& motor, unsigned long ahora) {
    const bool atacando = esAtaque(decision.tipo);
    if (decision.tipo == TipoAccion::Busqueda && veniaAtacando && calcularFreno(TiempoParoPerdida)) {
        enParo = true;
        inicioParo = ahora;
    }
    veniaAtacando = atacando;
    if (!enParo) {
        return false;
    }
    if (decision.tipo != TipoAccion::Busqueda || ahora - inicioParo >= duracionFreno) {
        enParo = false;
        return false;
    }
    mover(motor, frenoIzq, frenoDer, true);
    return true;
}

// Predicción de la posición con lo que hacen las ruedas y corrección al ver la
// línea (ver EstimadorBorde)
void ControlMovimiento::actualizarEstimador(const DecisionMovimiento& decision, unsigned long ahora) {
    // Como mucho una vez por milisegundo: en el ATmega cada actualización cuesta
    // varias funciones trigonométricas en coma flotante
    const unsigned long dtMs = ahora - ultimaPrediccion;
    bool cambio = dtMs > 0;
    if (dtMs > 0) {
        ultimaPrediccion = ahora;
        float empujon = 0;
        if (empujando) {
            empujon = EmpujonContacto;
        } else if (esAtaque(decision.tipo)) {
            empujon = EmpujonAtaque;
        } else if (decision.tipo == TipoAccion::DefensaIzq || decision.tipo == TipoAccion::DefensaDer) {
            empujon = EmpujonLateral;
        }
        // La primera llamada (dtMs enorme) solo fija el instante inicial
        if (dtMs < 1000) {
            estimador.predecir(ordenIzq, ordenDer, dtMs / 1000.0f, empujon);
        }
    }
    const bool borde = esBorde(decision.tipo);
    if (borde && !bordeAntes) {
        const int lado = decision.tipo == TipoAccion::EvadirBordeIzq ? -1
                       : decision.tipo == TipoAccion::EvadirBordeDer ? 1 : 0;
        estimador.lineaVista(lado, estIzq + estDer > 0);
        cambio = true;
    }
    bordeAntes = borde;
    if (cambio) {
        limiteEstimador = estimador.confiable() ? estimador.limiteAvance() : 255;
    }
}

bool ControlMovimiento::confirmar(bool detectando, unsigned long ahora, bool& activo, unsigned long& inicio) {
    if (!detectando) {
        activo = false;
        return false;
    }
    if (!activo) {
        activo = true;
        inicio = ahora;
    }
    return ahora - inicio >= ConfirmacionDeteccion;
}

// Rutina de inicio del round. Los giros siguen hasta tener al rival de frente
// (o su tiempo máximo); el avance, hasta ver al rival con cualquier sensor.
bool ControlMovimiento::continuarRutina(const DecisionMovimiento& decision, IMotor& motor, unsigned long ahora) {
    if (rutina == 0) {
        return false;
    }
    if (!rutinaEnCurso) {
        rutinaEnCurso = true;
        inicioRutina = ahora;
        detectaRutina = false;
        ladoDecidido = false;
        // Al acabar, la búsqueda sigue girando hacia el mismo lado
        sentidoBusqueda = ladoRutina;
    }
    const unsigned long t = ahora - inicioRutina;
    bool sigue = false;
    if (frenandoGiroRutina) {
        // Contragiro proporcional a lo que giraba: la inercia lo pasaría de largo
        sigue = ahora - inicioFrenoRutina < duracionFreno;
        if (sigue) {
            mover(motor, frenoIzq, frenoDer, true);
        } else {
            frenandoGiroRutina = false;
            rutina = 0;
        }
        return sigue;
    }
    if (rutina == 1 || rutina == 2) {
        // Una detección no termina el giro antes de su mínimo ni sin confirmarse
        const unsigned long maximo = rutina == 1 ? TiempoMaxGiroEspalda : TiempoMaxGiroLado;
        const unsigned long minimo = rutina == 1 ? TiempoMinimoGiroEspalda : TiempoMinimoGiroLado;
        const bool confirmado = confirmar(veDeFrente(decision), ahora, detectaRutina, inicioDetectaRutina);
        const bool encontrado = confirmado && t >= minimo;
        sigue = !encontrado && t < maximo;
        // Round 2: el primer sensor lateral que ve al rival decide el lado (el DIP3
        // solo vale mientras ninguno lo ve)
        if (rutina == 2 && !ladoDecidido &&
            (decision.tipo == TipoAccion::DefensaIzq || decision.tipo == TipoAccion::DefensaDer)) {
            ladoRutina = decision.tipo == TipoAccion::DefensaIzq ? -1 : 1;
            sentidoBusqueda = ladoRutina;
            ladoDecidido = true;
        }
        if (encontrado) {
            embestidaInicio = true;
            finEmbestidaInicio = ahora + TiempoEmbestidaInicio;
            if (calcularFreno(TiempoFrenadoRuedas)) {
                frenandoGiroRutina = true;
                inicioFrenoRutina = ahora;
                mover(motor, frenoIzq, frenoDer, true);
                return true;
            }
        }
        if (sigue && rutina == 2 && PivoteRound2) {
            // Pivota sobre la rueda del lado del rival: esa queda parada y la otra empuja
            mover(motor, ladoRutina > 0 ? VelocidadPivoteInicio : 0, ladoRutina > 0 ? 0 : VelocidadPivoteInicio, true);
        } else if (sigue) {
            mover(motor, ladoRutina * VelocidadGiroInicio, -ladoRutina * VelocidadGiroInicio, true);
        }
    } else if (rutina == 3) {
        // Sale pegado al borde mirando al centro: el rival solo puede aparecer por
        // delante (frontal o 45°). Los laterales apuntan fuera del dohyo, donde
        // hay gente y objetos, así que no cortan el avance.
        const bool porDelante = veDeFrente(decision) || decision.tipo == TipoAccion::CorregirIzq ||
                                decision.tipo == TipoAccion::CorregirDer;
        const bool confirmado = confirmar(porDelante, ahora, detectaRutina, inicioDetectaRutina);
        sigue = !(confirmado && t >= TiempoMinimoAvanceInicio) && t < TiempoAvanceInicio;
        if (sigue) {
            mover(motor, VelocidadAvanceInicio, VelocidadAvanceInicio);
        }
    }
    if (!sigue) {
        rutina = 0;
    }
    return sigue;
}

// Enemigo pegado de frente mientras el robot está en la línea: si lo está
// empujando o lo empujan a él, frenar o retroceder solo ayudaría al enemigo a
// sacarlo. Sigue empujando a fondo hasta que lo pierda de vista.
bool ControlMovimiento::resistirEnBorde(const DecisionMovimiento& decision, IMotor& motor) {
    if (!ResistirEnBorde || !esBorde(decision.tipo) || !empujando || fase != Fase::Libre) {
        return false;
    }
    buscando = false;
    giroLateral = 0;
    enParo = false;
    veniaAtacando = true;
    mover(motor, VelocidadEmpuje, VelocidadEmpuje, true);
    return true;
}

void ControlMovimiento::ejecutar(const DecisionMovimiento& decision, IMotor& motor, unsigned long ahora) {
    if (salidaSuaveUnSensor && ahora - inicioSalidaSuave >= TiempoSalidaSuaveUnSensor) {
        salidaSuaveUnSensor = false;
    }
    limiteSalidaSuave = salidaSuaveUnSensor ? VelocidadSalidaUnSensor : 255;
    if (primeraRampa) inicioMovimiento = ahora;
    const unsigned long desdeInicio = ahora - inicioMovimiento;
    limiteArranque = !conRutina && rampa > 0 && TiempoArranqueSuave > 0 && desdeInicio < TiempoArranqueSuave
        ? (long)VelocidadAtaque * desdeInicio / TiempoArranqueSuave : 255;
    const unsigned long dtRampa = primeraRampa ? 0 : ahora - ultimaRampa;
    primeraRampa = false;
    ultimaRampa = ahora;
    const unsigned long paso = dtRampa * (unsigned long)rampa;
    pasoRampa = paso > 255 ? 255 : (int)paso;
    estimarVelocidad(ahora);
    recordarLadoEnemigo(decision.tipo);

    // El ataque aumenta progresivamente mientras se mantiene alineado.
    // Perder al enemigo o iniciar una evasión reinicia la progresión.
    const bool atacando = esAtaque(decision.tipo) || (esBorde(decision.tipo) && decision.enemigoFrente);
    if (!atacando || fase != Fase::Libre) {
        viendoFrente = false;
        empujando = false;
    } else if (!empujando) {
        if (!veDeFrente(decision)) {
            viendoFrente = false;  // solo a 45°: aún no está alineado
        } else if (!viendoFrente) {
            viendoFrente = true;
            inicioFrente = ahora;
        }
        empujando = viendoFrente && ahora - inicioFrente >= TiempoEmbestida;
    }
    // El aviso de borde también limita el ataque: un objeto fuera puede activarlo.
    limitarAvance = decision.cercaBorde;
    if (UsarEstimadorBorde) {
        actualizarEstimador(decision, ahora);
    }

    if (resistirEnBorde(decision, motor)) {
        return;
    }

    switch (decision.tipo) {
    case TipoAccion::EvadirBordeIzq:
        iniciarEvasion(1, TiempoRetrocesoUnSensor, TiempoGiroEvasion, ahora, true);
        break;
    case TipoAccion::EvadirBordeDer:
        iniciarEvasion(-1, TiempoRetrocesoUnSensor, TiempoGiroEvasion, ahora, true);
        break;
    case TipoAccion::EvadirBordeAmbos:
        // Separarse del borde antes del giro lento hacia el último lado del enemigo
        iniciarEvasion(sentidoBusqueda, TiempoRetrocesoAmbos, TiempoGiroEvasionAmbos, ahora);
        break;
    default:
        break;
    }

    // El borde cancela la rutina de inicio
    if (rutina != 0 && (esBorde(decision.tipo) || fase != Fase::Libre)) {
        rutina = 0;
    }

    if (continuarEvasion(decision, motor, ahora)) {
        return;
    }

    if (continuarRutina(decision, motor, ahora)) {
        buscando = false;
        return;
    }

    // No dejar pausas o giros de búsqueda pendientes cuando los tres frontales
    // confirman el objetivo. Una evasión ya iniciada se completa antes de atacar.
    if (decision.ataqueDirecto && decision.tipo == TipoAccion::AtaqueFrontal) {
        enParo = false;
        giroLateral = 0;
        buscando = false;
        evasionCompleta = false;
    }

    if (decision.tipo != TipoAccion::Busqueda) {
        buscando = false;
    }

    if (continuarParo(decision, motor, ahora)) {
        return;
    }

    if (continuarGiroLateral(decision, motor, ahora)) {
        buscando = false;
        return;
    }

    // La embestida de la rutina dura mientras siga atacando y no se acabe su tiempo
    if (embestidaInicio && (!esAtaque(decision.tipo) || (long)(ahora - finEmbestidaInicio) >= 0)) {
        embestidaInicio = false;
    }
    int ataque = VelocidadAtaque;
    if (embestidaInicio) {
        ataque = VelocidadEmbestidaInicio;
    } else if (empujando || (viendoFrente && TiempoEmbestida == 0)) {
        ataque = VelocidadEmpuje;
    } else if (viendoFrente) {
        const unsigned long transcurrido = ahora - inicioFrente;
        const unsigned long progreso = transcurrido < TiempoEmbestida
                                      ? transcurrido : TiempoEmbestida;
        ataque += (long)(VelocidadEmpuje - VelocidadAtaque) * progreso / TiempoEmbestida;
    }
    switch (decision.tipo) {
    case TipoAccion::AtaqueFrontal:
        mover(motor, ataque, ataque);
        break;
    case TipoAccion::AjusteIzq:
        mover(motor, ataque * PorcentajeAjuste / 100, ataque);
        break;
    case TipoAccion::AjusteDer:
        mover(motor, ataque, ataque * PorcentajeAjuste / 100);
        break;
    case TipoAccion::CorregirIzq:
        // Pivota sobre la rueda izquierda (o curva, según Corregir45EnPivote)
        mover(motor, Corregir45EnPivote ? 0 : VelocidadCurva * ataque / VelocidadAtaque, ataque);
        break;
    case TipoAccion::CorregirDer:
        mover(motor, ataque, Corregir45EnPivote ? 0 : VelocidadCurva * ataque / VelocidadAtaque);
        break;
    case TipoAccion::DefensaIzq:
        girarHaciaLado(motor, -1);
        break;
    case TipoAccion::DefensaDer:
        girarHaciaLado(motor, 1);
        break;
    case TipoAccion::Busqueda:
    default:
        buscar(motor, ahora);
        break;
    }
}
