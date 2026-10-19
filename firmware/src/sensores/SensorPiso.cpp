#include <Arduino.h>
#include <stdlib.h>
#include "sensores/SensorPiso.h"
#include "configuracion/Parametros.h"

SensorPiso::SensorPiso(uint8_t pin, int negroInicial): pin(pin) {
    calibrar(negroInicial);
}

int SensorPiso::leer() const {
    return analogRead(pin);
}

void SensorPiso::calibrar() {
    long suma = 0;
    for (int i = 0; i < MuestrasCalibracionPiso; i++) {
        suma += analogRead(pin);
    }
    calibrar(suma / MuestrasCalibracionPiso);
}

void SensorPiso::calibrar(int negroMedido) {
    negro = negroMedido;
    margen = calcularMargen(negroMedido);
    enLinea = false;
}

int SensorPiso::calcularMargen(int negroMedido) {
    const int calculado = (long)negroMedido * PorcentajeUmbralLinea / 100;
    return calculado > UmbralLineaMinimo ? calculado : UmbralLineaMinimo;
}

bool SensorPiso::hayLinea(int lectura) {
    const int salida = margen * 70 / 100;
    const int diferencia = abs(lectura - negro);
    enLinea = diferencia > (enLinea ? salida : margen);
    return enLinea;
}

bool SensorPiso::cercaDeLinea(int lectura) const {
    return (long)abs(lectura - negro) * 100 > (long)margen * PorcentajeCercaBorde;
}
