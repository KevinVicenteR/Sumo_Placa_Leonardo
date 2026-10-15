#include "movimiento/ControlMovimiento.h"
#include "movimiento/RedControl.h"

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
    // La rutina puede haber terminado empezando un golpe: límites al día
    actualizarLimites(decision);

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
    mando.limites.golpe = ataque.golpeando();
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

    if (UsarPID && esAtaque(decision.tipo)) {
        // PID: gira hacia el rival en proporción a la corrección
        int u = pid.correccion(decision.errorRival, ahora);
        // Empujando: frenar mucho una rueda le quita fuerza al empuje
        constexpr int limiteEmpuje = (int)(LimitePIDEmpujando * ControlPID::Unidad + 0.5f);
        if (ataque.empujando()) {
            if (u > limiteEmpuje) u = limiteEmpuje;
            if (u < -limiteEmpuje) u = -limiteEmpuje;
        }
        int izq, der;
        if (PIDDiferencial) {
            // Acelera la exterior y frena la interior; si la exterior pasa del
            // máximo, baja las dos para mantener la diferencia
            const int giro = (int)((long)velocidad * u / ControlPID::Unidad);
            izq = velocidad + giro;
            der = velocidad - giro;
            const int exceso = (izq > der ? izq : der) - VelocidadMaxima;
            if (exceso > 0) {
                izq -= exceso;
                der -= exceso;
            }
        } else {
            izq = u < 0 ? (int)((long)velocidad * (ControlPID::Unidad + u) / ControlPID::Unidad) : velocidad;
            der = u > 0 ? (int)((long)velocidad * (ControlPID::Unidad - u) / ControlPID::Unidad) : velocidad;
        }
        moverConRed(decision, motor, ahora, izq, der);
        return;
    }

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
    case TipoAccion::EsquivarIzq:
        moverConRed(decision, motor, ahora, VelocidadEsquivaInterior, VelocidadEsquivaExterior);
        break;
    case TipoAccion::EsquivarDer:
        moverConRed(decision, motor, ahora, VelocidadEsquivaExterior, VelocidadEsquivaInterior);
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

void ControlMovimiento::moverConRed(const DecisionMovimiento& decision, IMotor& motor, unsigned long ahora,
                                    int izq, int der) {
    if (!UsarRedControl) {
        mando.mover(motor, izq, der);
        return;
    }
    // Ataque o esquiva nuevo (o tras una pausa): empieza a contar de cero
    if (!atacando || ahora - ultimoCicloRed > 50) {
        inicioAtaque = ahora;
        errorAnterior = decision.errorRival;
    }
    atacando = true;
    ultimoCicloRed = ahora;
    int cambio = (decision.errorRival - errorAnterior) / 125;
    if (cambio > 16) cambio = 16;
    if (cambio < -16) cambio = -16;
    const unsigned long tramos = (ahora - inicioAtaque) / 100;
    const int8_t x[redcontrol::NumEntradasRed] = {
        (int8_t)(decision.errorRival / 125),
        (int8_t)cambio,
        (int8_t)(ataque.empujando() ? 8 : 0),
        (int8_t)(tramos > 8 ? 8 : tramos),
        (int8_t)(limitadorBorde.limite() < 255 ? 8 : 0),
        (int8_t)(mando.velocidadAvanceEstimada() / 32),
        (int8_t)(decision.tipo == TipoAccion::EsquivarIzq ? -8 : decision.tipo == TipoAccion::EsquivarDer ? 8 : 0),
    };
    errorAnterior = decision.errorRival;
    int avance, giro;
    redcontrol::corregir(x, avance, giro);
    if (avance != 0 || giro != 0) {
        // Avance y giro de la orden de siempre, más la corrección de la red
        const int base = (izq + der) / 2 + avance;
        const int vuelta = (der - izq) / 2 + giro;
        izq = base - vuelta;
        der = base + vuelta;
        izq = izq > 255 ? 255 : izq < -255 ? -255 : izq;
        der = der > 255 ? 255 : der < -255 ? -255 : der;
    }
    mando.mover(motor, izq, der);
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
    pid = ControlPID();
    numEvasiones = 0;
    atacando = false;
}
