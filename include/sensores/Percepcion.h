// =============================================================================
// Percepcion.h — Todos los sensores del robot detrás de una sola clase
// -----------------------------------------------------------------------------
// Patrón Fachada (Facade): agrupa los 2 sensores de piso y los 5 de enemigo y
// devuelve en una sola llamada (leer) todo lo que el robot necesita saber.
// =============================================================================
#ifndef PERCEPCION_H
#define PERCEPCION_H

#include "sensores/IPercepcion.h"
#include "sensores/SensorPiso.h"
#include "sensores/SensorEnemigo.h"
#include "Pines.h"
#include "Parametros.h"

class Percepcion: public IPercepcion {
public:
    LecturasSensores leer() override;

    // Mide el negro del piso. Llamar con el robot ya colocado sobre el dohyo
    void calibrarPiso();
    // Fija o aprende la polaridad de los sensores de enemigo (ver
    // PolaridadSensoresEnemigo). Solo en diagnóstico: el rival puede estar delante
    void calibrarSensoresEnemigo();

    int negroIzquierdo() const { return pisoIzq.negroCalibrado(); }
    int negroDerecho() const { return pisoDer.negroCalibrado(); }
    int margenIzquierdo() const { return pisoIzq.margenLinea(); }
    int margenDerecho() const { return pisoDer.margenLinea(); }

private:
    // Polaridad guardada del sensor número "indice" (bit de MascaraEnemigosActivosHigh)
    static constexpr bool activoAlto(int indice) { return (MascaraEnemigosActivosHigh >> indice) & 1; }

    SensorPiso pisoIzq{S_PISO_IZQ, NegroPisoIzquierdo};
    SensorPiso pisoDer{S_PISO_DER, NegroPisoDerecho};

    SensorEnemigo latIzq{S_LAT_IZQ, activoAlto(0)};
    SensorEnemigo c45Izq{S_FRONT_IZQ, activoAlto(1)};
    SensorEnemigo frontal{S_FRONT_CEN, activoAlto(2)};
    SensorEnemigo c45Der{S_FRONT_DER, activoAlto(3)};
    SensorEnemigo latDer{S_LAT_DER, activoAlto(4)};
};

#endif
