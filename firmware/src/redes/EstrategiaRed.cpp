#include "redes/EstrategiaRed.h"
#include <Arduino.h>
#include "movimiento/ControlMovimiento.h"
#include "configuracion/ParametrosRedes.h"

DecisionMovimiento EstrategiaRed::elegirContraEnemigo(const LecturasSensores& l) {
    const unsigned long ahora = millis();
    const bool algo = l.latIzq || l.c45Izq || l.frontal || l.c45Der || l.latDer;
    if (algo) {
        ultimaVez = ahora;
    }
    if (!l.frontal || inicioFrontal == 0) {
        inicioFrontal = ahora;
    }
    const int8_t x[redlogica::NumEntradasRed] = {
        l.frontal, l.c45Izq, l.c45Der, l.latIzq, l.latDer,
        (int8_t)ladoUltimo,
        l.cercaBorde,
        !algo && ultimaVez != 0 && ahora - ultimaVez < RecuerdoRivalMs,
        ahora - inicioFrontal >= ContactoFrontalMs,
        contexto && contexto->limiteAvanceActual() < 255,
        esquivando,
        contexto && contexto->rutinaActual() != 0,
    };
    constexpr int minima = 0;
    TipoAccion accion = redlogica::Acciones[redlogica::elegir(x, minima, sumas)];
    if (esquivando && algo && ahora - inicioEsquiva < TiempoMinimoEsquivaRed) {
        accion = ultimaEsquiva;
    } else if (esEsquiva(accion)) {
        if (huboEsquiva && ahora - inicioEsquiva < TiempoRearmeEsquivaRed) {
            int mejor = 0;
            for (int k = 1; k < redlogica::SalidaBusqueda + 1; ++k) {
                if (sumas[k] > sumas[mejor]) mejor = k;
            }
            accion = sumas[mejor] >= minima ? redlogica::Acciones[mejor] : TipoAccion::Busqueda;
        } else {
            inicioEsquiva = ahora;
            huboEsquiva = true;
            ultimaEsquiva = accion;
        }
    }
    esquivando = esEsquiva(accion);
    if (l.c45Izq || l.latIzq) {
        ladoUltimo = 0;
    } else if (l.c45Der || l.latDer) {
        ladoUltimo = 1;
    }
    return {accion, l.frontal};
}

DecisionMovimiento EstrategiaRed::decidir(const LecturasSensores& l) {
    if (l.lineaIzq || l.lineaDer) {
        const TipoAccion tipo = l.lineaIzq && l.lineaDer ? TipoAccion::EvadirBordeAmbos
                              : l.lineaIzq ? TipoAccion::EvadirBordeIzq : TipoAccion::EvadirBordeDer;
        return {tipo, l.frontal, l.cercaBorde};
    }
    DecisionMovimiento d = elegirContraEnemigo(l);
    const bool centrado = l.c45Izq && l.frontal && l.c45Der;
    if (centrado && !esEsquiva(d.tipo)) d.tipo = TipoAccion::AtaqueFrontal;
    d.cercaBorde = l.cercaBorde;
    d.ataqueDirecto = centrado && d.tipo == TipoAccion::AtaqueFrontal;
    const int vistos = l.c45Izq + l.frontal + l.c45Der;
    d.errorRival = vistos ? (l.c45Der - l.c45Izq) * PosicionSensor45PID / vistos : 0;
    return d;
}
