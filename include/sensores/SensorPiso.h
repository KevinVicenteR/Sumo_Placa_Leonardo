// =============================================================================
// SensorPiso.h — Un sensor de piso (detecta la línea blanca del borde)
// -----------------------------------------------------------------------------
// Aprende cuánto lee sobre el negro del dohyo y considera "línea" cualquier
// lectura claramente distinta, sin importar si el sensor da valores más altos
// o más bajos sobre el blanco.
// =============================================================================
#ifndef SENSOR_PISO_H
#define SENSOR_PISO_H

#include <stdint.h>

class SensorPiso {
public:
    SensorPiso(uint8_t pin, int negroInicial);

    // Lectura analógica cruda (0-1023)
    int leer() const;
    // Mide el negro con el robot sobre el dohyo (promedio de varias lecturas)
    void calibrar();
    // Fija la referencia de negro a mano
    void calibrar(int negro);

    // ¿Hay línea bajo el sensor? Usa histéresis: entra al superar el margen y
    // solo sale al volver claramente al negro
    bool hayLinea(int lectura);
    // ¿La lectura empieza a alejarse del negro, sin llegar aún a la línea?
    bool cercaDeLinea(int lectura) const;

    int negroCalibrado() const { return negro; }
    int margenLinea() const { return margen; }

private:
    static int calcularMargen(int negro);

    uint8_t pin;
    int negro = 0;
    int margen = 0;
    bool enLinea = false;
};

#endif
