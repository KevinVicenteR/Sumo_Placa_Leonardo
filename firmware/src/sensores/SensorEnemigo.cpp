#include <Arduino.h>
#include "sensores/SensorEnemigo.h"
#include "configuracion/Parametros.h"

SensorEnemigo::SensorEnemigo(uint8_t pin, bool activoAlto)
    : pin(pin), activoAlto(activoAlto), filtro(TiempoRetencionEnemigo, ConfirmacionSensorEnemigo) {}

bool SensorEnemigo::mayoriaAlta() const {
    uint8_t altos = 0;
    altos += (digitalRead(pin) == HIGH) ? 1 : 0;
    altos += (digitalRead(pin) == HIGH) ? 1 : 0;
    altos += (digitalRead(pin) == HIGH) ? 1 : 0;
    return altos >= 2;
}

bool SensorEnemigo::detecta(unsigned long ahora) {
    return filtro.actualizar(mayoriaAlta() == activoAlto, ahora);
}

void SensorEnemigo::reiniciarFiltro() {
    filtro = RetencionDeteccion(TiempoRetencionEnemigo, ConfirmacionSensorEnemigo);
}
