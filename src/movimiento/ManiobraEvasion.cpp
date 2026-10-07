#include "movimiento/ManiobraEvasion.h"

void ManiobraEvasion::actualizarSalidaSuave(unsigned long ahora) {
    if (salidaSuave && ahora - inicioSalidaSuave >= TiempoSalidaSuaveUnSensor) {
        salidaSuave = false;
    }
    limiteSalida = salidaSuave ? VelocidadSalidaUnSensor : 255;
}

bool ManiobraEvasion::iniciar(int sentido, unsigned long duracionRet, unsigned long duracionGir,
                              unsigned long ahora, bool lineaUnSensor, bool veniaAvanzando) {
    if (fase != Fase::Libre) {
        // Ya evadiendo: si aparece el segundo sensor, pasa a la maniobra completa
        // sin reiniciar el tiempo máximo
        if (unSensor && !lineaUnSensor) {
            unSensor = false;
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
    duracionRetroceso = duracionRet;
    duracionGiro = duracionGir;
    sentidoGiro = sentido;
    return true;
}

bool ManiobraEvasion::continuar(const DecisionMovimiento& decision, MandoMotores& mando, IMotor& motor,
                                unsigned long ahora, BusquedaRival& busqueda) {
    const bool linea = esBorde(decision.tipo);

    // Línea otra vez a mitad de maniobra: volver a separarse
    const bool separandose = fase == Fase::Retrocediendo || fase == Fase::GirandoSalida;
    if (fase != Fase::Libre && !separandose && linea &&
        ahora - inicioEvasion < TiempoMaximoRecuperacionBorde) {
        fase = sinRetroceso ? Fase::GirandoSalida : Fase::Retrocediendo;
        inicioFase = ahora;
        duracionRetroceso = 0;
        negroContinuo = false;
    }

    // --- Fase 1: separarse de la línea ---
    if (fase == Fase::Retrocediendo || fase == Fase::GirandoSalida) {
        if (separarse(linea, mando, motor, ahora)) {
            return true;
        }
        // Girando ya se apartó del borde: sin pausa, sigue con el giro normal
        fase = fase == Fase::GirandoSalida ? Fase::Girando : Fase::Frenando;
        inicioFase = ahora;
    }

    // --- Fase 2: pausa con los motores parados ---
    if (fase == Fase::Frenando) {
        if (linea || ahora - inicioFase < TiempoFrenado) {
            mando.mover(motor, 0, 0, true);
            return true;
        }
        fase = Fase::Girando;
        inicioFase = ahora;
    }

    // --- Fase 3: girar en el sitio para apartarse del borde ---
    if (fase == Fase::Girando) {
        if (!linea && ahora - inicioFase < duracionGiro) {
            mando.mover(motor, sentidoGiro * VelocidadGiroEvasion,
                        -sentidoGiro * VelocidadGiroEvasion, true);
            return true;
        }
        fase = Fase::Asentando;
        inicioFase = ahora;
    }

    // --- Fase 4: otra pausa y fin ---
    if (fase == Fase::Asentando) {
        if (linea || ahora - inicioFase < TiempoAsentamientoEvasion) {
            mando.mover(motor, 0, 0, true);
            return true;
        }
        // Con un solo sensor: buscar hacia el lado del giro y salir despacio
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

bool ManiobraEvasion::separarse(bool linea, MandoMotores& mando, IMotor& motor, unsigned long ahora) {
    // Cuenta el tiempo seguido viendo negro
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
        // Gira en el sitio: el centro del robot no se mueve hacia el borde
        mando.mover(motor, sentidoGiro * VelocidadGiroEvasion, -sentidoGiro * VelocidadGiroEvasion, true);
    } else {
        const int velocidad = unSensor ? VelocidadRetrocesoUnSensor : VelocidadRetroceso;
        mando.mover(motor, -velocidad, -velocidad, true);
    }
    return true;
}
