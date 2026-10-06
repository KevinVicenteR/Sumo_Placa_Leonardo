#include <Arduino.h>
#include <stdlib.h>
#include "Percepcion.H"
#include "Pines.H"

bool Percepcion::mayoriaAlta(uint8_t pin) {
    uint8_t altos = 0;
    altos += (digitalRead(pin) == HIGH) ? 1 : 0;
    altos += (digitalRead(pin) == HIGH) ? 1 : 0;
    altos += (digitalRead(pin) == HIGH) ? 1 : 0;
    return altos >= 2;
}

uint8_t Percepcion::indiceEnemigo(uint8_t pin) {
    if (pin == S_LAT_IZQ) return 0;
    if (pin == S_FRONT_IZQ) return 1;
    if (pin == S_FRONT_CEN) return 2;
    if (pin == S_FRONT_DER) return 3;
    return 4;
}

bool Percepcion::lecturaDigitalMayoritaria(uint8_t pin) const {
    const bool activoAlto = (polaridadesAltas & (1 << indiceEnemigo(pin))) != 0;
    return mayoriaAlta(pin) == activoAlto;
}

void Percepcion::calibrarSensoresEnemigo() {
    if (PolaridadSensoresEnemigo != 0) {
        polaridadesAltas = PolaridadSensoresEnemigo > 0 ? 31 : 0;
    } else {
        const uint8_t pines[] = {S_LAT_IZQ, S_FRONT_IZQ, S_FRONT_CEN, S_FRONT_DER, S_LAT_DER};
        polaridadesAltas = 0;
        for (uint8_t i = 0; i < 5; ++i) {
            uint8_t altos = 0;
            for (uint8_t j = 0; j < 32; ++j) altos += mayoriaAlta(pines[i]) ? 1 : 0;
            if (altos < 16) polaridadesAltas |= (1 << i);
        }
    }
    retLatIzq = RetencionDeteccion(TiempoRetencionEnemigo);
    retC45Izq = RetencionDeteccion(TiempoRetencionEnemigo);
    retFrontal = RetencionDeteccion(TiempoRetencionEnemigo);
    retC45Der = RetencionDeteccion(TiempoRetencionEnemigo);
    retLatDer = RetencionDeteccion(TiempoRetencionEnemigo);
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

bool Percepcion::detectarLinea(SensorPiso& sensor, int lectura, unsigned long) {
    const int salida = sensor.margen * 70 / 100;
    const int diferencia = abs(lectura - sensor.negro);
    sensor.enLinea = diferencia > (sensor.enLinea ? salida : sensor.margen);
    return sensor.enLinea;
}

bool Percepcion::cercaDeLinea(const SensorPiso& sensor, int lectura) {
    return (long)abs(lectura - sensor.negro) * 100 > (long)sensor.margen * PorcentajeCercaBorde;
}

LecturasSensores Percepcion::leer() const {
    const unsigned long ahora = millis();
    LecturasSensores lecturas;
    const int izq = analogRead(S_PISO_IZQ);
    const int der = analogRead(S_PISO_DER);
    lecturas.lineaIzq = detectarLinea(pisoIzq, izq, ahora);
    lecturas.lineaDer = detectarLinea(pisoDer, der, ahora);
    lecturas.cercaBorde = cercaDeLinea(pisoIzq, izq) || cercaDeLinea(pisoDer, der);
    lecturas.latIzq = retLatIzq.actualizar(lecturaDigitalMayoritaria(S_LAT_IZQ), ahora);
    lecturas.c45Izq = retC45Izq.actualizar(lecturaDigitalMayoritaria(S_FRONT_IZQ), ahora);
    lecturas.frontal = retFrontal.actualizar(lecturaDigitalMayoritaria(S_FRONT_CEN), ahora);
    lecturas.c45Der = retC45Der.actualizar(lecturaDigitalMayoritaria(S_FRONT_DER), ahora);
    lecturas.latDer = retLatDer.actualizar(lecturaDigitalMayoritaria(S_LAT_DER), ahora);
    return lecturas;
}
