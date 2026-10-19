#ifndef PERCEPCION_H
#define PERCEPCION_H

#include "sensores/IPercepcion.h"
#include "sensores/SensorPiso.h"
#include "sensores/SensorEnemigo.h"
#include "configuracion/Pines.h"
#include "configuracion/Parametros.h"

class Percepcion: public IPercepcion {
public:
    LecturasSensores leer() override;

    void calibrarPiso();

    int negroIzquierdo() const { return pisoIzq.negroCalibrado(); }
    int negroDerecho() const { return pisoDer.negroCalibrado(); }
    int margenIzquierdo() const { return pisoIzq.margenLinea(); }
    int margenDerecho() const { return pisoDer.margenLinea(); }

private:
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
