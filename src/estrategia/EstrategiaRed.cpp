#include "estrategia/EstrategiaRed.h"
#include <Arduino.h>
#include "estrategia/RedPesos.h"
#include "Parametros.h"

DecisionMovimiento EstrategiaRed::elegirContraEnemigo(const LecturasSensores& l) {
    const unsigned long ahora = millis();
    const bool algo = l.latIzq || l.c45Izq || l.frontal || l.c45Der || l.latDer;
    if (algo) {
        ultimaVez = ahora;
    }
    if (!l.frontal) {
        inicioFrontal = ahora;
    }
    const int8_t x[redlogica::NumEntradasRed] = {
        l.frontal, l.c45Izq, l.c45Der, l.latIzq, l.latDer,
        (int8_t)ladoUltimo,
        l.cercaBorde,
        !algo && ultimaVez != 0 && ahora - ultimaVez < RecuerdoRivalMs,
        ahora - inicioFrontal >= ContactoFrontalMs,
        contexto && contexto->bordeDelante(),
        esquivando,
        contexto && contexto->enRutina(),
    };
    constexpr int minima = redpesos::sumaMinima(FuncionActivacion, UmbralActivacion);
    TipoAccion accion = redlogica::Acciones[redlogica::elegir(x, minima, sumas)];
    // Una esquiva necesita varios ciclos para formar un arco. Evita alternar
    // ataque/esquiva cada lectura y deja una ventana para volver a atacar.
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
    // Recordar el lado donde se vio al rival por última vez
    if (l.c45Izq || l.latIzq) {
        ladoUltimo = 0;
    } else if (l.c45Der || l.latDer) {
        ladoUltimo = 1;
    }
    return {accion, l.frontal};
}
