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
        int limite = 255;
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
    // Se decide por lo que el robot está haciendo realmente (velocidad estimada) y
    // no por la última orden: justo tras un giro aún gira por la inercia aunque ya
    // se le haya ordenado avanzar, y justo tras avanzar aún se desliza hacia delante
    // aunque se le haya ordenado girar.
    const long izq = estIzq / 256, der = estDer / 256;
    evasionEnSitio = (izq > 0 && der < 0) || (izq < 0 && der > 0);
    if (evasionEnSitio) {
        duracionRet = 0;
        duracionGir = TiempoGiroEvasionEnSitio;
    } else if (escapeArco) {
        duracionRet = duracionRet == TiempoRetrocesoAmbos ? TiempoArcoEscapeAmbos : TiempoArcoEscape;
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
            mover(motor, frenoIzq, frenoDer, true);
            return true;
        }
        fase = Fase::Retrocediendo;
        inicioFase += duracionFreno;
    }

    if (fase == Fase::Retrocediendo) {
        const unsigned long retrocedido = ahora - inicioFase;
        if (escapeArco && !evasionEnSitio) {
            // Retrocede en arco: la rueda del lado hacia el que hay que girar va
            // más rápido hacia atrás, así el frente se aparta del borde mientras
            // el robot se aleja de él, sin pararse a girar
            if (retrocedido < duracionRetroceso) {
                const int exterior = -VelocidadRetroceso;
                const int interior = -VelocidadRetroceso * PorcentajeArcoEscape / 100;
                mover(motor, sentidoGiro > 0 ? interior : exterior, sentidoGiro > 0 ? exterior : interior, true);
                return true;
            }
            evasionCompleta = decision.tipo == TipoAccion::Busqueda;
            fase = Fase::Libre;
            return false;
        }
        if (retrocedido < duracionRetroceso) {
            mover(motor, -VelocidadRetroceso, -VelocidadRetroceso, true);
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
            mover(motor, 0, 0, true);
            return true;
        }
        fase = Fase::Girando;
        inicioFase += TiempoFrenado;
    }

    if (fase == Fase::Girando) {
        // Completar el giro aunque se vea un objeto fuera del dohyo.
        const unsigned long girado = ahora - inicioFase;
        if (girado < duracionGiro) {
            mover(motor, sentidoGiro * VelocidadGiroEvasion, -sentidoGiro * VelocidadGiroEvasion, true);
            return true;
        }
        // En un escape en el sitio no se ignora blanco al terminar el tiempo.
        // Detenerse hasta que los sensores vuelvan al negro antes de avanzar.
        if (esBorde(decision.tipo)) {
            mover(motor, 0, 0, true);
            return true;
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
    const unsigned long tramo = (ahora - inicioBusqueda) / TiempoArcoBusqueda;
    const int sentido = tramo % 2 == 0 ? sentidoBusqueda : -sentidoBusqueda;
    const int exterior = VelocidadAvance;
    const int interior = VelocidadAvance * PorcentajeArcoBusqueda / 100;
    mover(motor, sentido > 0 ? exterior : interior, sentido > 0 ? interior : exterior);
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
        }
    }
    if (giroLateral == 0) {
        return false;
    }
    const bool otroLado = (giroLateral < 0 && decision.tipo == TipoAccion::CorregirDer) ||
                          (giroLateral > 0 && decision.tipo == TipoAccion::CorregirIzq);
    if (veDeFrente(decision) || otroLado || ahora - inicioGiroLateral >= TiempoMaxGiroLateral) {
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
    const unsigned long dtRampa = primeraRampa ? 1000 : ahora - ultimaRampa;
    primeraRampa = false;
    ultimaRampa = ahora;
    const unsigned long paso = dtRampa * (unsigned long)rampa;
    pasoRampa = paso > 255 ? 255 : (int)paso;
    estimarVelocidad(ahora);
    recordarLadoEnemigo(decision.tipo);

    // Ataque en dos fases. Primero se acomoda: se acerca a VelocidadAtaque
    // corrigiendo hasta tener al enemigo en el sensor frontal. Cuando lo mantiene
    // de frente TiempoEmbestida sin interrupción, va con todo (VelocidadEmpuje) y
    // sigue así mientras dure el ataque, aunque el enemigo baile entre el frontal
    // y los de 45°. Durante una evasión no cuenta aunque lo siga viendo: el robot
    // no está avanzando hacia él.
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

    const int ataque = empujando ? VelocidadEmpuje : VelocidadAtaque;
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
        mover(motor, VelocidadCurva * ataque / VelocidadAtaque, ataque);
        break;
    case TipoAccion::CorregirDer:
        mover(motor, ataque, VelocidadCurva * ataque / VelocidadAtaque);
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
