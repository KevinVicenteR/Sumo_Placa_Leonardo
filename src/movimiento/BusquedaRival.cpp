#include "movimiento/BusquedaRival.h"

void BusquedaRival::recordarLadoEnemigo(TipoAccion tipo) {
    switch (tipo) {
    case TipoAccion::AjusteIzq:
    case TipoAccion::CorregirIzq:
    case TipoAccion::DefensaIzq:
        sentidoGiro = -1;
        break;
    case TipoAccion::AjusteDer:
    case TipoAccion::CorregirDer:
    case TipoAccion::DefensaDer:
        sentidoGiro = 1;
        break;
    default:
        break;
    }
}

void BusquedaRival::ejecutar(MandoMotores& mando, IMotor& motor, unsigned long ahora) {
    if (patron == 1) {
        buscarEnArcos(mando, motor, ahora);
        return;
    }

    // Patrón 0. Tras una evasión empieza avanzando; si no, empieza girando
    if (!buscando) {
        buscando = true;
        inicioBusqueda = evasionCompleta ? ahora : ahora - TiempoAvanceBusqueda - TiempoPausaBusqueda;
        evasionCompleta = false;
    }
    // Posición dentro del ciclo: avance | pausa | giro | pausa
    const unsigned long finAvance = TiempoAvanceBusqueda;
    const unsigned long inicioGiro = finAvance + TiempoPausaBusqueda;
    const unsigned long finGiro = inicioGiro + TiempoGiroBusqueda;
    const unsigned long t = (ahora - inicioBusqueda) % (finGiro + TiempoPausaBusqueda);
    if (t < finAvance) {
        avanzarPorPulsos(mando, motor, t);
    } else if (t >= inicioGiro && t < finGiro) {
        mando.mover(motor, sentidoGiro * VelocidadGiroBusqueda, -sentidoGiro * VelocidadGiroBusqueda);
    } else {
        mando.mover(motor, 0, 0);
    }
}

void BusquedaRival::buscarEnArcos(MandoMotores& mando, IMotor& motor, unsigned long ahora) {
    if (!buscando) {
        buscando = true;
        inicioBusqueda = ahora;
        evasionCompleta = false;
    }
    // Cada tramo es un arco; la rueda interior pasa gradualmente a ser la exterior
    const unsigned long transcurrido = ahora - inicioBusqueda;
    const unsigned long tramo = transcurrido / TiempoArcoBusqueda;
    const unsigned long progreso = transcurrido % TiempoArcoBusqueda;
    const int interior = VelocidadAvance * PorcentajeArcoBusqueda / 100;
    const int cambio = (long)(VelocidadAvance - interior) * progreso / TiempoArcoBusqueda;
    int izq = tramo % 2 == 0 ? VelocidadAvance - cambio : interior + cambio;
    int der = tramo % 2 == 0 ? interior + cambio : VelocidadAvance - cambio;
    // Los arcos empiezan hacia el lado donde se vio al rival
    if (sentidoGiro < 0) {
        const int temporal = izq;
        izq = der;
        der = temporal;
    }
    // Al empezar, acelera poco a poco
    if (TiempoArranqueBusqueda > 0 && transcurrido < TiempoArranqueBusqueda) {
        izq = (long)izq * transcurrido / TiempoArranqueBusqueda;
        der = (long)der * transcurrido / TiempoArranqueBusqueda;
    }
    mando.mover(motor, izq, der);
}

// Avance del patrón 0: continuo o en pulsos, recto o en zigzag
void BusquedaRival::avanzarPorPulsos(MandoMotores& mando, IMotor& motor, unsigned long t) {
    if (TiempoPulsoAvance > 0 && t % (TiempoPulsoAvance + TiempoPausaPulso) >= TiempoPulsoAvance) {
        mando.mover(motor, 0, 0);
        return;
    }
    const int interior = VelocidadAvance * PorcentajeArcoZigzag / 100;
    const bool haciaIzq = TiempoArcoZigzag > 0 && (t / TiempoArcoZigzag) % 2 == 0;
    if (haciaIzq) {
        mando.mover(motor, interior, VelocidadAvance);
    } else {
        mando.mover(motor, VelocidadAvance, interior);
    }
}
