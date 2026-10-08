#include "movimiento/ControlMovimiento.h"

void ControlMovimiento::ejecutar(const DecisionMovimiento& decision, IMotor& motor, unsigned long ahora) {
    // --- 1. Preparar el ciclo: actualizar el estado de cada maniobra ---
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

    // --- 2. Borde: tiene prioridad sobre todo ---
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

    // --- 3. Rutina de inicio del round ---
    if (rutina.continuar(decision, mando, motor, ahora, busqueda, ataque)) {
        busqueda.detener();
        return;
    }

    // Rival centrado en los tres sensores: cualquier otra maniobra se cancela
    if (decision.ataqueDirecto && decision.tipo == TipoAccion::AtaqueFrontal) {
        paro.cancelar();
        giroLateral.cancelar();
        busqueda.cancelar();
    }
    if (decision.tipo != TipoAccion::Busqueda) {
        busqueda.detener();
    }

    // --- 4. Frenado al perder al rival ---
    if (paro.continuar(decision, mando, motor, ahora)) {
        return;
    }

    // --- 5. Giro hacia un rival visto de lado ---
    if (giroLateral.continuar(decision, mando, motor, ahora)) {
        busqueda.detener();
        return;
    }

    // --- 6. Acción normal ---
    ejecutarAccion(decision, motor, ahora);
}

void ControlMovimiento::actualizarLimites(const DecisionMovimiento& decision) {
    mando.limites.salidaSuave = evasion.limiteSalidaSuave();
    mando.limites.cercaBorde = decision.cercaBorde;
    mando.limites.estimador = limitadorBorde.limite();
    mando.limites.empujando = ataque.empujando();
}

void ControlMovimiento::empezarEvasionSiHayBorde(const DecisionMovimiento& decision, unsigned long ahora) {
    // Retroceder solo si venía avanzando hacia la línea (ver AvanceMinimoParaRetroceder)
    const int velocidad = mando.velocidadAvanceEstimada();
    const bool avanzaba = !SalirGirandoSiNoAvanza || velocidad >= AvanceMinimoParaRetroceder;
    // Cuanto más rápido llega, más retrocede (ver RetrocesoExtraAVelocidadMaxima)
    const unsigned long extra = velocidad > 0 ? RetrocesoExtraAVelocidadMaxima * velocidad / 255 : 0;
    bool empezo = false;
    switch (decision.tipo) {
    case TipoAccion::EvadirBordeIzq:
        // Línea a la izquierda: retroceso corto y giro a la derecha
        empezo = evasion.iniciar(1, TiempoRetrocesoUnSensor + extra, TiempoGiroEvasion, ahora, true, avanzaba);
        break;
    case TipoAccion::EvadirBordeDer:
        empezo = evasion.iniciar(-1, TiempoRetrocesoUnSensor + extra, TiempoGiroEvasion, ahora, true, avanzaba);
        break;
    case TipoAccion::EvadirBordeAmbos:
        // Línea de frente: gira hacia el último lado donde se vio al rival
        empezo = evasion.iniciar(busqueda.sentido(), TiempoRetrocesoAmbos + extra, TiempoGiroEvasionAmbos, ahora,
                                 false, avanzaba);
        break;
    default:
        break;
    }
    // Una evasión nueva interrumpe todo lo demás
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
    // Rueda interior al corregir hacia un rival visto a 45°
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
