#ifndef PERCEPCION_H
#define PERCEPCION_H

#include <stdint.h>
#include "LecturasSensores.H"
#include "IPercepcion.H"
#include "RetencionDeteccion.H"
#include "Parametros.H"

class Percepcion: public IPercepcion {
public:
    LecturasSensores leer() const override;
    // Debe llamarse con el robot sobre el negro del dohyo
    void calibrarPiso();

private:
    // Referencia de negro de un sensor de piso, que se corrige sola si la
    // calibración se hizo con el sensor sobre la línea
    struct SensorPiso {
        int negro = 0;
        int margen = UmbralLineaMinimo;
        bool enLinea = false;
        unsigned long inicioLinea = 0;
    };

    static bool lecturaDigitalMayoritaria(uint8_t pin);
    static int promedioAnalogico(uint8_t pin, int muestras);
    static int margenLinea(int negro);
    static void calibrar(SensorPiso& sensor, int negro);
    static bool detectarLinea(SensorPiso& sensor, int lectura, unsigned long ahora);

    mutable SensorPiso pisoIzq;
    mutable SensorPiso pisoDer;

    mutable RetencionDeteccion retLatIzq{TiempoRetencionEnemigo};
    mutable RetencionDeteccion retC45Izq{TiempoRetencionEnemigo};
    mutable RetencionDeteccion retFrontal{TiempoRetencionEnemigo};
    mutable RetencionDeteccion retC45Der{TiempoRetencionEnemigo};
    mutable RetencionDeteccion retLatDer{TiempoRetencionEnemigo};
};

#endif
