#include "maniobras/ManiobraEvasion.h"

void ManiobraEvasion::actualizarSalidaSuave(unsigned long ahora) {
    if (salidaSuave && ahora - inicioSalidaSuave >= TiempoSalidaSuaveUnSensor) {
        salidaSuave = false;
    }
    limiteSalida = salidaSuave ? VelocidadSalidaUnSensor : 255;
}

bool ManiobraEvasion::iniciar(int sentido, unsigned long duracionRet, unsigned long duracionGir,
                              unsigned long ahora, bool lineaUnSensor, bool veniaAvanzando) {
    if (fase != Fase::Libre) {
        if (unSensor && !lineaUnSensor) {
            unSensor = false;
            if (mejorarRetroceso && duracionRet > duracionRetroceso) duracionRetroceso = duracionRet;
            duracionGiro = duracionGir;
            negroContinuo = false;
        }
        return false;
    }
    unSensor = lineaUnSensor;
    sinRetroceso = !veniaAvanzando;
    salidaSuave = false;
    limiteSalida = 255;
    fase = sinRetroceso ? Fase::GirandoSalida : Fase::Retrocediendo;
    inicioFase = ahora;
    inicioEvasion = ahora;
    negroContinuo = false;
    viendoRival = false;
    duracionRetroceso = duracionRet;
    duracionGiro = duracionGir;
    sentidoGiro = sentido;
    return true;
}

bool ManiobraEvasion::continuar(const DecisionMovimiento& decision, MandoMotores& mando, IMotor& motor,
                                unsigned long ahora, BusquedaRival& busqueda) {
    const bool linea = esBorde(decision.tipo);

    const bool yaSeparado = fase == Fase::Girando || fase == Fase::Asentando;
    const bool puedeCortar = yaSeparado && (!huboCorte || ahora - ultimoCorte >= EsperaEntreCortesEvasion);
    if (puedeCortar && rivalConfirmado(decision, linea, ahora)) {
        huboCorte = true;
        ultimoCorte = ahora;
        fase = Fase::Libre;
        salidaSuave = false;
        limiteSalida = 255;
        return false;
    }

    const bool separandose = fase == Fase::Retrocediendo || fase == Fase::GirandoSalida;
    if (fase != Fase::Libre && !separandose && linea &&
        ahora - inicioEvasion < TiempoMaximoRecuperacionBorde) {
        fase = sinRetroceso ? Fase::GirandoSalida : Fase::Retrocediendo;
        inicioFase = ahora;
        duracionRetroceso = 0;
        negroContinuo = false;
    }

    if (fase == Fase::Retrocediendo || fase == Fase::GirandoSalida) {
        if (separarse(linea, mando, motor, ahora)) {
            return true;
        }
        fase = fase == Fase::GirandoSalida ? Fase::Girando : Fase::Frenando;
        inicioFase = ahora;
    }

    if (fase == Fase::Frenando) {
        if (reintentarSiAtascado(linea, mando, motor, ahora)) {
            return true;
        }
        if (linea || ahora - inicioFase < TiempoFrenado) {
            mando.mover(motor, 0, 0, true);
            return true;
        }
        fase = Fase::Girando;
        inicioFase = ahora;
    }

    if (fase == Fase::Girando) {
        if (!linea && ahora - inicioFase < duracionGiro) {
            girar(mando, motor);
            return true;
        }
        fase = Fase::Asentando;
        inicioFase = ahora;
    }

    if (fase == Fase::Asentando) {
        if (reintentarSiAtascado(linea, mando, motor, ahora)) {
            return true;
        }
        if (linea || ahora - inicioFase < TiempoAsentamientoEvasion) {
            mando.mover(motor, 0, 0, true);
            return true;
        }
        if (unSensor) {
            busqueda.fijarSentido(sentidoGiro);
            salidaSuave = true;
            inicioSalidaSuave = ahora;
            limiteSalida = VelocidadSalidaUnSensor;
        }
        busqueda.terminoEvasion(decision.tipo == TipoAccion::Busqueda);
        fase = Fase::Libre;
    }

    return false;
}

bool ManiobraEvasion::rivalConfirmado(const DecisionMovimiento& decision, bool linea,
                                      unsigned long ahora) {
    const bool rival = CortarEvasionAlVerRival && !linea && decision.tipo != TipoAccion::Busqueda;
    if (!rival) {
        viendoRival = false;
        return false;
    }
    if (!viendoRival) {
        viendoRival = true;
        inicioRival = ahora;
    }
    return ahora - inicioRival >= TiempoConfirmarRivalEnEvasion;
}

bool ManiobraEvasion::separarse(bool linea, MandoMotores& mando, IMotor& motor, unsigned long ahora) {
    if (linea) {
        negroContinuo = false;
    } else if (!negroContinuo) {
        negroContinuo = true;
        inicioNegro = ahora;
    }
    const unsigned long separacion = unSensor ? TiempoSeparacionUnSensor : TiempoSeparacionBorde;
    const bool separado = negroContinuo && ahora - inicioNegro >= separacion &&
                          ahora - inicioFase >= duracionRetroceso;
    const bool agotado = ahora - inicioEvasion >= TiempoMaximoRecuperacionBorde;
    if (separado || agotado) {
        return false;
    }
    if (fase == Fase::GirandoSalida) {
        girar(mando, motor);
    } else {
        const int velocidad = unSensor ? VelocidadRetrocesoUnSensor : VelocidadRetroceso;
        mando.mover(motor, -velocidad, -velocidad, true);
    }
    return true;
}

bool ManiobraEvasion::reintentarSiAtascado(bool linea, MandoMotores& mando, IMotor& motor, unsigned long ahora) {
    if (!linea || ahora - inicioFase < TiempoMaxParadoEnLinea) {
        return false;
    }
    sinRetroceso = false;
    unSensor = true;
    fase = Fase::Retrocediendo;
    inicioFase = ahora;
    inicioEvasion = ahora - (TiempoMaximoRecuperacionBorde - TiempoReintentoRetroceso);
    duracionRetroceso = 0;
    negroContinuo = false;
    separarse(linea, mando, motor, ahora);
    return true;
}

void ManiobraEvasion::girar(MandoMotores& mando, IMotor& motor) const {
    const int v = sentidoGiro * VelocidadGiroEvasion;
    if (GirarSobreUnaRueda && PivoteEnEvasion) {
        mando.mover(motor, v < 0 ? 2 * v : 0, v > 0 ? -2 * v : 0, true);
    } else {
        mando.mover(motor, v, -v, true, true);
    }
}
