#include <Arduino.h>
#include "Percepcion.H"
#include "Pines.H"

bool Percepcion::lecturaDigitalMayoritaria(uint8_t pin) {
    uint8_t activos = 0;
    activos += (digitalRead(pin) == HIGH) ? 1 : 0;
    activos += (digitalRead(pin) == HIGH) ? 1 : 0;
    activos += (digitalRead(pin) == HIGH) ? 1 : 0;
    return activos >= 2;
}

int Percepcion::promedioAnalogico(uint8_t pin, int muestras) {
    long suma = 0;
    for (int i = 0; i < muestras; i++) {
        suma += analogRead(pin);
    }
    return suma / muestras;
}

int Percepcion::margenLinea(int negro) {
    const int margen = (long)negro * PorcentajeUmbralLinea / 100;
    return margen > UmbralLineaMinimo ? margen : UmbralLineaMinimo;
}

void Percepcion::calibrar(SensorPiso& sensor, int negro) {
    sensor.negro = negro;
    sensor.margen = margenLinea(negro);
    sensor.enLinea = false;
}

void Percepcion::calibrarPiso() {
    calibrar(pisoIzq, promedioAnalogico(S_PISO_IZQ, MuestrasCalibracionPiso));
    calibrar(pisoDer, promedioAnalogico(S_PISO_DER, MuestrasCalibracionPiso));
}

// Línea = lectura claramente distinta del negro calibrado, sin importar si el
// sensor da valores más altos o más bajos sobre el blanco
bool Percepcion::detectarLinea(SensorPiso& sensor, int lectura, unsigned long ahora) {
    if (abs(lectura - sensor.negro) <= sensor.margen) {
        sensor.enLinea = false;
        return false;
    }
    if (!sensor.enLinea) {
        sensor.enLinea = true;
        sensor.inicioLinea = ahora;
    } else if (ahora - sensor.inicioLinea > TiempoLineaSospechosa) {
        // Con el robot en marcha una línea de 1 cm no se ve tanto tiempo seguido:
        // el negro se midió con el sensor sobre la línea y se vuelve a medir
        calibrar(sensor, lectura);
        return false;
    }
    return true;
}

LecturasSensores Percepcion::leer() const {
    const unsigned long ahora = millis();
    LecturasSensores lecturas;
    lecturas.lineaIzq = detectarLinea(pisoIzq, analogRead(S_PISO_IZQ), ahora);
    lecturas.lineaDer = detectarLinea(pisoDer, analogRead(S_PISO_DER), ahora);
    lecturas.latIzq = retLatIzq.actualizar(lecturaDigitalMayoritaria(S_LAT_IZQ), ahora);
    lecturas.c45Izq = retC45Izq.actualizar(lecturaDigitalMayoritaria(S_FRONT_IZQ), ahora);
    lecturas.frontal = retFrontal.actualizar(lecturaDigitalMayoritaria(S_FRONT_CEN), ahora);
    lecturas.c45Der = retC45Der.actualizar(lecturaDigitalMayoritaria(S_FRONT_DER), ahora);
    lecturas.latDer = retLatDer.actualizar(lecturaDigitalMayoritaria(S_LAT_DER), ahora);
    return lecturas;
}
