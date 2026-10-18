#include <Arduino.h>
#include "sensores/SensorEnemigo.h"
#include "Parametros.h"

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

void SensorEnemigo::aprenderPolaridad() {
    // Sin nada delante, el nivel habitual es "no detecta"; detectar es el contrario
    uint8_t altos = 0;
    for (uint8_t i = 0; i < 32; ++i) altos += mayoriaAlta() ? 1 : 0;
    activoAlto = altos < 16;
}

void SensorEnemigo::reiniciarFiltro() {
    filtro = RetencionDeteccion(TiempoRetencionEnemigo, ConfirmacionSensorEnemigo);
}
