#include "ControlMovimiento.H"
#include "Parametros.H"

namespace {
long magnitud(long v) { return v < 0 ? -v : v; }

bool esBorde(TipoAccion tipo) {
    return tipo == TipoAccion::EvadirBordeIzq || tipo == TipoAccion::EvadirBordeDer ||
           tipo == TipoAccion::EvadirBordeAmbos;
}

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
    if (fase != Fase::Libre) {
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
        if (esBorde(decision.tipo) || ahora - inicioFase < TiempoFrenado) {
            mover(motor, 0, 0, true);
            return true;
        }
        fase = Fase::Girando;
        inicioFase = ahora;
    }

    if (fase == Fase::Girando) {
        if (!esBorde(decision.tipo) && ahora - inicioFase < duracionGiro) {
            mover(motor, sentidoGiro * VelocidadGiroEvasion,
                  -sentidoGiro * VelocidadGiroEvasion, true);
            return true;
        }
        fase = Fase::Asentando;
        inicioFase = ahora;
    }

    if (fase == Fase::Asentando) {
        if (esBorde(decision.tipo) || ahora - inicioFase < TiempoAsentamientoEvasion) {
            mover(motor, 0, 0, true);
            return true;
        }
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
    const int cambio = (long)(VelocidadAvance - interior) * progreso / TiempoArcoBusqueda;
    int izq = tramo % 2 == 0 ? VelocidadAvance - cambio : interior + cambio;
    int der = tramo % 2 == 0 ? interior + cambio : VelocidadAvance - cambio;
    if (sentidoBusqueda < 0) {
        const int temporal = izq;
        izq = der;
        der = temporal;
    }
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
        inicioBusqueda = evasionCompleta ? ahora : ahora - TiempoAvanceBusqueda - TiempoPausaBusqueda;
        evasionCompleta = false;
    }
    const unsigned long finAvance = TiempoAvanceBusqueda;
    const unsigned long inicioGiro = finAvance + TiempoPausaBusqueda;
    const unsigned long finGiro = inicioGiro + TiempoGiroBusqueda;
    const unsigned long t = (ahora - inicioBusqueda) % (finGiro + TiempoPausaBusqueda);
    if (t < finAvance) {
        mover(motor, VelocidadAvance, VelocidadAvance);
    } else if (t >= inicioGiro && t < finGiro) {
        mover(motor, sentidoBusqueda * VelocidadGiroBusqueda, -sentidoBusqueda * VelocidadGiroBusqueda);
    } else {
        mover(motor, 0, 0);
    }
}

void ControlMovimiento::girarHaciaLado(IMotor& motor, int sentido) {
    if (GiroLateralEnRueda) {
        mover(motor, sentido > 0 ? VelocidadRuedaPivote : 0, sentido > 0 ? 0 : VelocidadRuedaPivote);
    } else {
        mover(motor, sentido * VelocidadPivoteLateral, -sentido * VelocidadPivoteLateral);
    }
}

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

void ControlMovimiento::actualizarEstimador(const DecisionMovimiento& decision, unsigned long ahora) {
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

bool ControlMovimiento::continuarRutina(const DecisionMovimiento& decision, IMotor& motor, unsigned long ahora) {
    if (rutina == 0) {
        return false;
    }
    if (!rutinaEnCurso) {
        rutinaEnCurso = true;
        inicioRutina = ahora;
        detectaRutina = false;
        ladoDecidido = false;
        sentidoBusqueda = ladoRutina;
    }
    const unsigned long t = ahora - inicioRutina;
    bool sigue = false;
    if (frenandoGiroRutina) {
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
        const unsigned long maximo = rutina == 1 ? TiempoMaxGiroEspalda : TiempoMaxGiroLado;
        const unsigned long minimo = rutina == 1 ? TiempoMinimoGiroEspalda : TiempoMinimoGiroLado;
        const bool confirmado = confirmar(veDeFrente(decision), ahora, detectaRutina, inicioDetectaRutina);
        const bool encontrado = confirmado && t >= minimo;
        sigue = !encontrado && t < maximo;
        if (rutina == 2 && !ladoDecidido &&
            (decision.tipo == TipoAccion::DefensaIzq || decision.tipo == TipoAccion::DefensaDer)) {
            ladoRutina = decision.tipo == TipoAccion::DefensaIzq ? -1 : 1;
            sentidoBusqueda = ladoRutina;
            ladoDecidido = true;
        }
        if (encontrado) {
            embestidaInicio = true;
            ataqueDeRutina = true;
            finEmbestidaInicio = ahora + TiempoEmbestidaInicio;
            if (calcularFreno(TiempoFrenadoRuedas)) {
                frenandoGiroRutina = true;
                inicioFrenoRutina = ahora;
                mover(motor, frenoIzq, frenoDer, true);
                return true;
            }
        }
        if (sigue && rutina == 2) {
            mover(motor, ladoRutina > 0 ? VelocidadPivoteInicio : 0, ladoRutina > 0 ? 0 : VelocidadPivoteInicio, true);
        } else if (sigue) {
            mover(motor, ladoRutina * VelocidadGiroInicio, -ladoRutina * VelocidadGiroInicio, true);
        }
    } else if (rutina == 3) {
        const bool porDelante = veDeFrente(decision) || decision.tipo == TipoAccion::CorregirIzq ||
                                decision.tipo == TipoAccion::CorregirDer;
        const bool esperando = EsperarRound3 && t >= TiempoAvanceInicio;
        const bool visto = esperando ? decision.tipo != TipoAccion::Busqueda : porDelante;
        const bool confirmado = confirmar(visto, ahora, detectaRutina, inicioDetectaRutina);
        const unsigned long fin = TiempoAvanceInicio + (EsperarRound3 ? TiempoEsperaRound3 : 0);
        sigue = !(confirmado && t >= TiempoMinimoAvanceInicio) && t < fin;
        if (sigue && esperando) {
            mover(motor, 0, 0, true);
        } else if (sigue) {
            mover(motor, VelocidadAvanceInicio, VelocidadAvanceInicio);
        }
    }
    if (!sigue) {
        rutina = 0;
    }
    return sigue;
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

    const bool atacando = esAtaque(decision.tipo) || (esBorde(decision.tipo) && decision.enemigoFrente);
    if (!atacando || fase != Fase::Libre) {
        viendoFrente = false;
        empujando = false;
    } else if (!empujando) {
        if (!veDeFrente(decision)) {
            viendoFrente = false;
        } else if (!viendoFrente) {
            viendoFrente = true;
            inicioFrente = ahora;
        }
        empujando = viendoFrente && ahora - inicioFrente >= TiempoEmbestida;
    }
    limitarAvance = decision.cercaBorde;
    if (UsarEstimadorBorde) {
        actualizarEstimador(decision, ahora);
    }

    switch (decision.tipo) {
    case TipoAccion::EvadirBordeIzq:
        iniciarEvasion(1, TiempoRetrocesoUnSensor, TiempoGiroEvasion, ahora, true);
        break;
    case TipoAccion::EvadirBordeDer:
        iniciarEvasion(-1, TiempoRetrocesoUnSensor, TiempoGiroEvasion, ahora, true);
        break;
    case TipoAccion::EvadirBordeAmbos:
        iniciarEvasion(sentidoBusqueda, TiempoRetrocesoAmbos, TiempoGiroEvasionAmbos, ahora);
        break;
    default:
        break;
    }

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

    if (embestidaInicio && (!esAtaque(decision.tipo) || (long)(ahora - finEmbestidaInicio) >= 0)) {
        embestidaInicio = false;
    }
    if (ataqueDeRutina && !esAtaque(decision.tipo) && rutina == 0) {
        ataqueDeRutina = false;
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
    if (ataqueDeRutina && !embestidaInicio && ataque > VelocidadAtaqueRound12) {
        ataque = VelocidadAtaqueRound12;
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
        mover(motor, 0, ataque);
        break;
    case TipoAccion::CorregirDer:
        mover(motor, ataque, 0);
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

void ControlMovimiento::reiniciar() {
    fase = Fase::Libre;
    inicioFase = inicioEvasion = inicioNegro = 0;
    negroContinuo = evasionUnSensor = salidaSuaveUnSensor = false;
    inicioSalidaSuave = 0;
    limiteSalidaSuave = 255;
    duracionRetroceso = duracionGiro = 0;
    sentidoGiro = 1;
    ordenIzq = ordenDer = 0;
    estIzq = estDer = 0;
    ultimaEstimacion = 0;
    frenoIzq = frenoDer = 0;
    duracionFreno = 0;
    veniaAtacando = enParo = false;
    inicioParo = 0;
    viendoFrente = empujando = limitarAvance = false;
    inicioFrente = 0;
    pasoRampa = 255;
    ultimaRampa = 0;
    primeraRampa = true;
    inicioMovimiento = 0;
    limiteArranque = limiteEstimador = 255;
    estimador = EstimadorBorde();
    ultimaPrediccion = 0;
    bordeAntes = false;
    giroLateral = 0;
    inicioGiroLateral = 0;
    rutina = 0;
    ladoRutina = 1;
    rutinaEnCurso = conRutina = embestidaInicio = ataqueDeRutina = frenandoGiroRutina = false;
    inicioFrenoRutina = finEmbestidaInicio = inicioRutina = 0;
    ladoDecidido = detectaRutina = detectaGiroLateral = false;
    inicioDetectaRutina = inicioDetectaGiroLateral = 0;
    sentidoBusqueda = 1;
    buscando = evasionCompleta = false;
    inicioBusqueda = 0;
}
