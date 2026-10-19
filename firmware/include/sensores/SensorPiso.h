#ifndef SENSOR_PISO_H
#define SENSOR_PISO_H

#include <stdint.h>

class SensorPiso {
public:
    SensorPiso(uint8_t pin, int negroInicial);

    int leer() const;
    void calibrar();
    void calibrar(int negro);

    bool hayLinea(int lectura);
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
