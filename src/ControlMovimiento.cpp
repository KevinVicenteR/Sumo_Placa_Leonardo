#include "ControlMovimiento.H"
#include "Parametros.H"

namespace {
long magnitud(long v) { return v < 0 ? -v : v; }
}

void ControlMovimiento::mover(IMotor& motor, int izq, int der) {
    ordenIzq = izq;
    ordenDer = der;
    sentidoEnSitio = (izq > 0 && der < 0) ? 1 : (izq < 0 && der > 0) ? -1 : 0;
    if (sentidoEnSitio == 0 && (izq != 0 || der != 0)) {
        huboAvance = true;
        ultimoAvance = ahoraActual;
    }
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
    frenoIzq = -izq * VelocidadMaxima / mayor;
    frenoDer = -der * VelocidadMaxima / mayor;
    duracionFreno = duracionMaxima * mayor / VelocidadMaxima;
    return true;
}

void ControlMovimiento::iniciarEvasion(int sentido, unsigned long duracionRet,
                                       unsigned long duracionGir, unsigned long ahora) {
    // Durante una evasión en el sitio el centro no se mueve: los sensores pueden
    // volver a cruzar la línea al girar, pero no hay riesgo y se termina el giro
    if (evasionEnSitio && fase != Fase::Libre) {
        return;
    }
    // Si la línea aparece mientras gira en el sitio, el robot puede estar de lado o
    // de espaldas al borde y retroceder lo sacaría. Sin retroceder, gira alejándose
    // del sensor que tocó la línea: como el primero en tocarla es el que va por
    // delante en el giro, el frente queda mirando hacia dentro tras unos 80°.
    // (Avanzando no se aplica: el frente mira al borde y retroceder lo aleja.)
    // Poco después de avanzar o retroceder, el robot aún se desliza por la inercia
    evasionEnSitio = sentidoEnSitio != 0 && (!huboAvance || ahora - ultimoAvance >= TiempoAsentarGiro);
    if (evasionEnSitio) {
        duracionRet = 0;
        duracionGir = TiempoGiroEvasionEnSitio;
    }
    if (fase == Fase::FrenandoRuedas) {
        return;
    }
    enParo = false;
    veniaAtacando = false;
    // Línea nueva mientras avanzaba o pivotaba: primero se frena cada rueda en
    // proporción a lo que iba haciendo. Retroceder con las dos ruedas a la vez
    // mantendría el giro que traía (la rueda rápida tarda más en parar) y la
    // parte trasera se balancearía hacia afuera.
    if (fase == Fase::Libre && !evasionEnSitio && TiempoFrenadoRuedas > 0) {
        if (calcularFreno(TiempoFrenadoRuedas)) {
            fase = Fase::FrenandoRuedas;
            inicioFase = ahora;
            duracionRetroceso = duracionRet;
            duracionGiro = duracionGir;
            sentidoGiro = sentido;
            buscando = false;
            evasionCompleta = false;
            giroLateral = 0;
            return;
        }
    }
    // Mientras se siga viendo la línea, la maniobra se reinicia y el retroceso se prolonga
    fase = Fase::Retrocediendo;
    inicioFase = ahora;
    duracionRetroceso = duracionRet;
    duracionGiro = duracionGir;
    sentidoGiro = sentido;
    buscando = false;
    evasionCompleta = false;
    giroLateral = 0;
}

bool ControlMovimiento::continuarEvasion(const DecisionMovimiento& decision, IMotor& motor, unsigned long ahora) {
    if (fase == Fase::FrenandoRuedas) {
        if (ahora - inicioFase < duracionFreno) {
            mover(motor, frenoIzq, frenoDer);
            return true;
        }
        fase = Fase::Retrocediendo;
        inicioFase += duracionFreno;
    }

    if (fase == Fase::Retrocediendo) {
        if (ahora - inicioFase < duracionRetroceso) {
            mover(motor, -VelocidadRetroceso, -VelocidadRetroceso);
            return true;
        }
        // Cada fase empieza cuando termina la anterior, aunque se note un ciclo después
        fase = duracionRetroceso > 0 ? Fase::Frenando : Fase::Girando;
        inicioFase += duracionRetroceso;
    }

    if (fase == Fase::Frenando) {
        // Sin frenar, la rueda que debe invertir tarda más que la otra y el giro
        // sale en curva hacia atrás, acercando el robot al borde
        if (ahora - inicioFase < TiempoFrenado) {
            mover(motor, 0, 0);
            return true;
        }
        fase = Fase::Girando;
        inicioFase += TiempoFrenado;
    }

    if (fase == Fase::Girando) {
        // Si un sensor de enemigo lo detecta durante el giro, se aborta para atacar,
        // pero no antes de haber girado lo mínimo para alejarse del borde
        const unsigned long girado = ahora - inicioFase;
        const bool enemigoVisto = decision.tipo != TipoAccion::Busqueda &&
                                  decision.tipo != TipoAccion::EvadirBordeIzq &&
                                  decision.tipo != TipoAccion::EvadirBordeDer &&
                                  decision.tipo != TipoAccion::EvadirBordeAmbos;
        if (girado < duracionGiro && (!enemigoVisto || girado < TiempoMinimoGiroEvasion)) {
            mover(motor, sentidoGiro * VelocidadGiroEvasion, -sentidoGiro * VelocidadGiroEvasion);
            return true;
        }
        evasionCompleta = decision.tipo == TipoAccion::Busqueda;
        fase = Fase::Libre;
    }

    return false;
}

void ControlMovimiento::recordarLadoEnemigo(TipoAccion tipo) {
    switch (tipo) {
    case TipoAccion::CorregirIzq:
    case TipoAccion::DefensaIzq:
        sentidoBusqueda = -1;
        break;
    case TipoAccion::CorregirDer:
    case TipoAccion::DefensaDer:
        sentidoBusqueda = 1;
        break;
    default:
        break;
    }
}

void ControlMovimiento::buscar(IMotor& motor, unsigned long ahora) {
    if (!buscando) {
        buscando = true;
        // Solo se empieza avanzando si una evasión completa dejó al robot mirando
        // hacia el centro; si no, podría estar de frente al borde y se gira primero
        inicioBusqueda = evasionCompleta ? ahora : ahora - TiempoAvanceBusqueda;
        evasionCompleta = false;
    }
    // Ciclo: avance corto -> giro en el sitio
    const unsigned long t = (ahora - inicioBusqueda) % (TiempoAvanceBusqueda + TiempoGiroBusqueda);
    if (t < TiempoAvanceBusqueda) {
        mover(motor, VelocidadAvance, VelocidadAvance);
    } else {
        mover(motor, sentidoBusqueda * VelocidadGiroBusqueda, -sentidoBusqueda * VelocidadGiroBusqueda);
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
        }
    }
    if (giroLateral == 0) {
        return false;
    }
    const bool otroLado = (giroLateral < 0 && decision.tipo == TipoAccion::CorregirDer) ||
                          (giroLateral > 0 && decision.tipo == TipoAccion::CorregirIzq);
    if (decision.tipo == TipoAccion::AtaqueFrontal || otroLado || ahora - inicioGiroLateral >= TiempoMaxGiroLateral) {
        giroLateral = 0;
        return false;
    }
    girarHaciaLado(motor, giroLateral);
    return true;
}

// Si iba atacando y el enemigo desaparece (lo esquivó), frena en seco en vez de
// seguir lanzado: a toda velocidad el robot no alcanzaría a parar al ver la línea
bool ControlMovimiento::continuarParo(const DecisionMovimiento& decision, IMotor& motor, unsigned long ahora) {
    const bool atacando = decision.tipo == TipoAccion::AtaqueFrontal ||
                          decision.tipo == TipoAccion::CorregirIzq ||
                          decision.tipo == TipoAccion::CorregirDer;
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
    mover(motor, frenoIzq, frenoDer);
    return true;
}

void ControlMovimiento::ejecutar(const DecisionMovimiento& decision, IMotor& motor, unsigned long ahora) {
    ahoraActual = ahora;
    estimarVelocidad(ahora);
    recordarLadoEnemigo(decision.tipo);

    switch (decision.tipo) {
    case TipoAccion::EvadirBordeIzq:
        iniciarEvasion(1, TiempoRetroceso, TiempoGiroEvasion, ahora);
        break;
    case TipoAccion::EvadirBordeDer:
        iniciarEvasion(-1, TiempoRetroceso, TiempoGiroEvasion, ahora);
        break;
    case TipoAccion::EvadirBordeAmbos:
        // De frente al borde: media vuelta hacia donde se vio al enemigo por última vez
        iniciarEvasion(sentidoBusqueda, TiempoRetrocesoAmbos, TiempoGiroEvasionAmbos, ahora);
        break;
    default:
        break;
    }

    if (continuarEvasion(decision, motor, ahora)) {
        return;
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

    switch (decision.tipo) {
    case TipoAccion::AtaqueFrontal:
        mover(motor, VelocidadAtaque, VelocidadAtaque);
        break;
    case TipoAccion::CorregirIzq:
        mover(motor, VelocidadCurva, VelocidadAtaque);
        break;
    case TipoAccion::CorregirDer:
        mover(motor, VelocidadAtaque, VelocidadCurva);
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
