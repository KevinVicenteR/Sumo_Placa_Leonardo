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
    // Averigua si los sensores de enemigo dan HIGH o LOW al detectar (ver
    // PolaridadSensoresEnemigo). Debe llamarse con el robot ya colocado.
    void calibrarSensoresEnemigo();
    bool enemigoActivoAlto() const { return activoAlto; }

private:
    // Referencia fija de negro: calibrar exclusivamente antes de arrancar
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

    bool activoAlto = PolaridadSensoresEnemigo >= 0;
    mutable SensorPiso pisoIzq;
    mutable SensorPiso pisoDer;

    mutable RetencionDeteccion retLatIzq{TiempoRetencionEnemigo};
    mutable RetencionDeteccion retC45Izq{TiempoRetencionEnemigo};
    mutable RetencionDeteccion retFrontal{TiempoRetencionEnemigo};
    mutable RetencionDeteccion retC45Der{TiempoRetencionEnemigo};
    mutable RetencionDeteccion retLatDer{TiempoRetencionEnemigo};
};

#endif
