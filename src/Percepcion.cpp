#include <Arduino.h>
#include <stdlib.h>
#include "Percepcion.H"
#include "Pines.H"

// El pin está en HIGH en al menos 2 de 3 lecturas (filtra parpadeos)
bool Percepcion::mayoriaAlta(uint8_t pin) {
    uint8_t altos = 0;
    altos += (digitalRead(pin) == HIGH) ? 1 : 0;
    altos += (digitalRead(pin) == HIGH) ? 1 : 0;
    altos += (digitalRead(pin) == HIGH) ? 1 : 0;
    return altos >= 2;
}

bool Percepcion::lecturaDigitalMayoritaria(uint8_t pin) const {
    return mayoriaAlta(pin) == activoAlto;
}

void Percepcion::calibrarSensoresEnemigo() {
    if (PolaridadSensoresEnemigo != 0) {
        activoAlto = PolaridadSensoresEnemigo > 0;
        return;
    }
    // Los dos laterales a la vez no pueden estar viendo al enemigo: si los dos
    // están en HIGH, HIGH es «no veo nada». Si difieren (enemigo a un lado) no se
    // puede saber y se supone HIGH al detectar.
    const bool izq = mayoriaAlta(S_LAT_IZQ);
    const bool der = mayoriaAlta(S_LAT_DER);
    activoAlto = !(izq && der);
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
bool Percepcion::detectarLinea(SensorPiso& sensor, int lectura, unsigned long) {
    // Histeresis: entrar inmediatamente y salir solo al volver claramente al negro.
    // Nunca recalibrar en marcha: blanco persistente sigue siendo borde.
    const int salida = sensor.margen * 70 / 100;
    const int diferencia = abs(lectura - sensor.negro);
    sensor.enLinea = diferencia > (sensor.enLinea ? salida : sensor.margen);
    return sensor.enLinea;
}

// La lectura ya se aleja del negro una fracción del margen de línea: el sensor
// empieza a tener blanco debajo (o el robot asoma por el borde)
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
