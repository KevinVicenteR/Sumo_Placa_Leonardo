#include "movimiento/ControlMovimiento.h"
#include "configuracion/ParametrosRedes.h"

void ControlMovimiento::ejecutar(const DecisionMovimiento& decision, IMotor& motor, unsigned long ahora) {
    mejorasEsteCiclo = mejorasActivas();
    evasion.configurarModo4(mejorasEsteCiclo && (competencia != 3 || MejorarRecuperacionModo3));
    evasion.actualizarSalidaSuave(ahora);
    mando.nuevoCiclo(ahora, rutina.elegida());
    busqueda.recordarLadoEnemigo(decision.tipo);
    ataque.actualizarContacto(decision, evasion.activa(), ahora);
    if (UsarEstimadorBorde) {
        limitadorBorde.actualizar(decision, mando, ataque.empujando(), ahora);
    }
    actualizarLimites(decision);

    if (resistirEnBorde(decision, motor)) {
        return;
    }

    empezarEvasionSiHayBorde(decision, ahora);
    if (rutina.actual() != 0 && (esBorde(decision.tipo) || evasion.activa())) {
        rutina.cancelar();
    }
    actualizarLimites(decision);
    const bool evadiendo = evasion.continuar(decision, mando, motor, ahora, busqueda);
    actualizarLimites(decision);
    if (evadiendo) {
        return;
    }

    if (rutina.continuar(decision, mando, motor, ahora, busqueda, ataque)) {
        busqueda.detener();
        return;
    }

    if (decision.ataqueDirecto && decision.tipo == TipoAccion::AtaqueFrontal) {
        paro.cancelar();
        giroLateral.cancelar();
        busqueda.cancelar();
    }
    if (decision.tipo != TipoAccion::Busqueda) {
        busqueda.detener();
    }

    if (paro.continuar(decision, mando, motor, ahora)) {
        return;
    }

    if (giroLateral.continuar(decision, mando, motor, ahora)) {
        busqueda.detener();
        return;
    }

    ejecutarAccion(decision, motor, ahora);
}

void ControlMovimiento::actualizarLimites(const DecisionMovimiento& decision) {
    mando.limites.salidaSuave = evasion.limiteSalidaSuave();
    mando.limites.cercaBorde = decision.cercaBorde;
    mando.limites.estimador = limitadorBorde.limite();
    mando.limites.empujando = ataque.empujando();
}

void ControlMovimiento::empezarEvasionSiHayBorde(const DecisionMovimiento& decision, unsigned long ahora) {
    const int velocidad = mando.velocidadAvanceEstimada();
    const bool avanzaba = !SalirGirandoSiNoAvanza || velocidad >= AvanceMinimoParaRetroceder;
    const unsigned long extra = velocidad > 0 ? RetrocesoExtraAVelocidadMaxima * velocidad / 255 : 0;
    bool empezo = false;
    switch (decision.tipo) {
    case TipoAccion::EvadirBordeIzq:
        empezo = evasion.iniciar(1, TiempoRetrocesoUnSensor + extra, TiempoGiroEvasion, ahora, true, avanzaba);
        break;
    case TipoAccion::EvadirBordeDer:
        empezo = evasion.iniciar(-1, TiempoRetrocesoUnSensor + extra, TiempoGiroEvasion, ahora, true, avanzaba);
        break;
    case TipoAccion::EvadirBordeAmbos:
        empezo = evasion.iniciar(busqueda.sentido(), TiempoRetrocesoAmbos + extra, TiempoGiroEvasionAmbos, ahora,
                                 false, avanzaba);
        break;
    default:
        break;
    }
    if (empezo) {
        numEvasiones++;
        paro.olvidar();
        busqueda.cancelar();
        giroLateral.cancelar();
        ataque.olvidarFrente();
    }
}

bool ControlMovimiento::resistirEnBorde(const DecisionMovimiento& decision, IMotor& motor) {
    if (!ResistirEnBorde || !esBorde(decision.tipo) || !ataque.empujando() || evasion.activa()) {
        return false;
    }
    busqueda.detener();
    giroLateral.cancelar();
    paro.marcarAtaque();
    mando.mover(motor, VelocidadEmpuje, VelocidadEmpuje, true);
    return true;
}

void ControlMovimiento::ejecutarAccion(const DecisionMovimiento& decision, IMotor& motor, unsigned long ahora) {
    const int velocidad = ataque.velocidad(decision, ahora);
    if (mejorasEsteCiclo && (esAtaque(decision.tipo) || esEsquiva(decision.tipo))) {
        controlRed.ejecutar(decision, mando, motor, ahora, velocidad, ataque.empujando(), limitadorBorde.limite(), competencia);
        return;
    }
    const int interior45 = Corregir45EnPivote ? 0 : VelocidadCurva * velocidad / VelocidadAtaque;

    switch (decision.tipo) {
    case TipoAccion::AtaqueFrontal:
        mando.mover(motor, velocidad, velocidad);
        break;
    case TipoAccion::AjusteIzq:
        mando.mover(motor, velocidad * PorcentajeAjuste / 100, velocidad);
        break;
    case TipoAccion::AjusteDer:
        mando.mover(motor, velocidad, velocidad * PorcentajeAjuste / 100);
        break;
    case TipoAccion::CorregirIzq:
        mando.mover(motor, interior45, velocidad);
        break;
    case TipoAccion::CorregirDer:
        mando.mover(motor, velocidad, interior45);
        break;
    case TipoAccion::DefensaIzq:
        GiroLateral::girarHacia(mando, motor, -1);
        break;
    case TipoAccion::DefensaDer:
        GiroLateral::girarHacia(mando, motor, 1);
        break;
    case TipoAccion::Busqueda:
    default:
        busqueda.ejecutar(mando, motor, ahora);
        break;
    }
}

void ControlMovimiento::reiniciar() {
    competencia = 0;
    mejorasEsteCiclo = false;
    ++generacion;
    controlRed.reiniciar();
    mando = MandoMotores(rampa);
    evasion = ManiobraEvasion();
    rutina = RutinaInicio();
    busqueda = BusquedaRival(patron);
    giroLateral = GiroLateral();
    paro = ParoPerdida();
    ataque = ControlAtaque();
    limitadorBorde = LimitadorBorde();
    numEvasiones = 0;
}
