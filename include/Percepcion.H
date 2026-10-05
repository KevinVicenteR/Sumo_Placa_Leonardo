#ifndef PERCEPCION_H
#define PERCEPCION_H

#include <stdint.h>
#include "LecturasSensores.H"
#include "IPercepcion.H"
#include "RetencionDeteccion.H"
#include "Parametros.H"

class Percepcion: public IPercepcion {
public:
    Percepcion() {
        calibrar(pisoIzq, NegroPisoIzquierdo);
        calibrar(pisoDer, NegroPisoDerecho);
    }
    LecturasSensores leer() const override;
    void calibrarPiso();
    void calibrarSensoresEnemigo();
    int negroIzquierdo() const { return pisoIzq.negro; }
    int negroDerecho() const { return pisoDer.negro; }
    int margenIzquierdo() const { return pisoIzq.margen; }
    int margenDerecho() const { return pisoDer.margen; }

private:
    struct SensorPiso {
        int negro = 0;
        int margen = UmbralLineaMinimo;
        bool enLinea = false;
    };

    bool lecturaDigitalMayoritaria(uint8_t pin) const;
    static bool mayoriaAlta(uint8_t pin);
    static int promedioAnalogico(uint8_t pin, int muestras);
    static int margenLinea(int negro);
    static void calibrar(SensorPiso& sensor, int negro);
    static bool detectarLinea(SensorPiso& sensor, int lectura, unsigned long ahora);
    static bool cercaDeLinea(const SensorPiso& sensor, int lectura);

    uint8_t polaridadesAltas = MascaraEnemigosActivosHigh;
    static uint8_t indiceEnemigo(uint8_t pin);
    mutable SensorPiso pisoIzq;
    mutable SensorPiso pisoDer;

    mutable RetencionDeteccion retLatIzq{TiempoRetencionEnemigo, ConfirmacionSensorEnemigo};
    mutable RetencionDeteccion retC45Izq{TiempoRetencionEnemigo, ConfirmacionSensorEnemigo};
    mutable RetencionDeteccion retFrontal{TiempoRetencionEnemigo, ConfirmacionSensorEnemigo};
    mutable RetencionDeteccion retC45Der{TiempoRetencionEnemigo, ConfirmacionSensorEnemigo};
    mutable RetencionDeteccion retLatDer{TiempoRetencionEnemigo, ConfirmacionSensorEnemigo};
};

#endif
