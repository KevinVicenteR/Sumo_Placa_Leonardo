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

LecturasSensores Percepcion::leer() const {
    const unsigned long ahora = millis();
    LecturasSensores lecturas;
    lecturas.lineaIzq = (analogRead(S_PISO_IZQ) < UmbralLinea);
    lecturas.lineaDer = (analogRead(S_PISO_DER) < UmbralLinea);
    lecturas.latIzq = retLatIzq.actualizar(lecturaDigitalMayoritaria(S_LAT_IZQ), ahora);
    lecturas.c45Izq = retC45Izq.actualizar(lecturaDigitalMayoritaria(S_FRONT_IZQ), ahora);
    lecturas.frontal = retFrontal.actualizar(lecturaDigitalMayoritaria(S_FRONT_CEN), ahora);
    lecturas.c45Der = retC45Der.actualizar(lecturaDigitalMayoritaria(S_FRONT_DER), ahora);
    lecturas.latDer = retLatDer.actualizar(lecturaDigitalMayoritaria(S_LAT_DER), ahora);
    return lecturas;
}
