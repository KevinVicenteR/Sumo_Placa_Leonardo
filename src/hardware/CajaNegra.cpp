#include <Arduino.h>
#include "hardware/CajaNegra.h"

#ifdef __AVR__
#include <EEPROM.h>
#else
// Fuera del Arduino (simulador) no hay EEPROM: se emula en RAM
namespace {
uint8_t eepromSimulada[1024];
}
#endif

void CajaNegra::escribir(int direccion, const void* datos, int tamano) {
    const uint8_t* bytes = static_cast<const uint8_t*>(datos);
    for (int i = 0; i < tamano; i++) {
#ifdef __AVR__
        EEPROM.update(direccion + i, bytes[i]);  // solo escribe los bytes que cambian
#else
        eepromSimulada[direccion + i] = bytes[i];
#endif
    }
}

void CajaNegra::leerBytes(int direccion, void* datos, int tamano) {
    uint8_t* bytes = static_cast<uint8_t*>(datos);
    for (int i = 0; i < tamano; i++) {
#ifdef __AVR__
        bytes[i] = EEPROM.read(direccion + i);
#else
        bytes[i] = eepromSimulada[direccion + i];
#endif
    }
}

CajaNegra::Cabecera CajaNegra::leerCabecera() const {
    Cabecera cabecera;
    leerBytes(0, &cabecera, sizeof cabecera);
    if (cabecera.firma != Firma) {
        cabecera = {Firma, 0};  // EEPROM sin caja negra: empieza desde cero
    }
    return cabecera;
}

void CajaNegra::empezar(uint8_t dip, int negroIzq, int negroDer, unsigned long ahora) {
    actual = RegistroCombate{};
    actual.numero = leerCabecera().siguiente;
    actual.dip = dip;
    actual.negroIzq = negroIzq;
    actual.negroDer = negroDer;
    inicio = ahora;
    for (uint8_t i = 0; i < 5; i++) detectando[i] = false;
    enCurso = true;
    guardado = false;
}

void CajaNegra::observar(const LecturasSensores& l, unsigned int evasiones, unsigned long ahora) {
    if (!enCurso) return;
    const unsigned long decimas = (ahora - inicio) / 100;
    actual.duracionDecimas = decimas > 65535 ? 65535 : decimas;
    actual.evasiones = evasiones;
    guardado = false;

    // Detección continua más larga de cada sensor de enemigo
    const bool ve[5] = {l.latIzq, l.c45Izq, l.frontal, l.c45Der, l.latDer};
    for (uint8_t i = 0; i < 5; i++) {
        if (!ve[i]) {
            detectando[i] = false;
            continue;
        }
        if (!detectando[i]) {
            detectando[i] = true;
            inicioDeteccion[i] = ahora;
        }
        const unsigned long d = (ahora - inicioDeteccion[i]) / 100;
        const uint8_t duracion = d > 255 ? 255 : d;
        if (duracion > actual.maxEnemigo[i]) actual.maxEnemigo[i] = duracion;
    }
}

void CajaNegra::anotarInterrupcion() {
    if (enCurso && actual.interrupciones < 255) {
        actual.interrupciones++;
        guardado = false;
    }
}

void CajaNegra::guardar() {
    if (!enCurso || guardado) return;
    // El combate número n va en la posición n % NumRegistros (anillo)
    const int direccion = sizeof(Cabecera) + (actual.numero % NumRegistros) * sizeof(RegistroCombate);
    escribir(direccion, &actual, sizeof actual);
    const Cabecera cabecera = {Firma, (uint16_t)(actual.numero + 1)};
    escribir(0, &cabecera, sizeof cabecera);
    guardado = true;
}

bool CajaNegra::leer(uint8_t i, RegistroCombate& registro) const {
    const Cabecera cabecera = leerCabecera();
    if (i >= NumRegistros || i >= cabecera.siguiente) return false;
    const uint16_t numero = cabecera.siguiente - 1 - i;
    leerBytes(sizeof(Cabecera) + (numero % NumRegistros) * sizeof(RegistroCombate), &registro, sizeof registro);
    return registro.numero == numero;
}

void CajaNegra::borrar() {
    const Cabecera cabecera = {Firma, 0};
    escribir(0, &cabecera, sizeof cabecera);
    enCurso = false;
}
