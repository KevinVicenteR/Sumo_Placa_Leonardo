// CajaNegra.h — Registro de los últimos combates en la EEPROM
// Como la caja negra de un avión: durante el combate anota en RAM lo que pasa y,
// con el robot parado (al recibir STOP), lo guarda en la EEPROM, que no se borra
// al apagar. Después se lee con el modo diagnóstico (tecla 'r').
// Escribir en la EEPROM tarda unos 3 ms por byte: nunca se hace en pleno combate.
#ifndef CAJA_NEGRA_H
#define CAJA_NEGRA_H

#include <stdint.h>
#include "sensores/LecturasSensores.h"

// Lo que se guarda de cada combate (20 bytes)
struct RegistroCombate {
    uint16_t numero;           // combate número (cuenta desde que se borró la caja)
    uint8_t dip;               // interruptores DIP al empezar (rutina elegida)
    uint8_t interrupciones;    // paradas cortas de la señal de START (interferencias)
    uint16_t negroIzq;         // negro calibrado de cada sensor de piso: si se
    uint16_t negroDer;         // parece al blanco, calibró sobre la línea
    uint16_t duracionDecimas;  // duración del combate (décimas de segundo)
    uint16_t evasiones;        // veces que evadió el borde
    uint8_t maxEnemigo[5];     // detección continua más larga de cada sensor de
                               // enemigo (décimas de s, máx. 25,5 s): muy larga = cegado
    uint8_t reservado;
};

class CajaNegra {
public:
    static constexpr uint8_t NumRegistros = 8;

    // Empieza a anotar un combate nuevo
    void empezar(uint8_t dip, int negroIzq, int negroDer, unsigned long ahora);
    // Un ciclo de combate: actualiza duraciones y evasiones
    void observar(const LecturasSensores& lecturas, unsigned int evasiones, unsigned long ahora);
    // Interrupción corta de la señal de START
    void anotarInterrupcion();
    // Guarda el combate en curso en la EEPROM (llamar solo con el robot parado)
    void guardar();

    // Lee el registro i (0 = el más reciente). Devuelve false si no existe
    bool leer(uint8_t i, RegistroCombate& registro) const;
    // Borra todos los registros
    void borrar();

private:
    struct Cabecera {
        uint16_t firma;        // indica que la EEPROM ya tiene una caja negra
        uint16_t siguiente;    // número del próximo combate
    };
    static constexpr uint16_t Firma = 0x534D;  // "SM"

    Cabecera leerCabecera() const;
    static void escribir(int direccion, const void* datos, int tamano);
    static void leerBytes(int direccion, void* datos, int tamano);

    RegistroCombate actual{};
    bool enCurso = false;
    bool guardado = true;
    unsigned long inicio = 0;
    unsigned long inicioDeteccion[5] = {0, 0, 0, 0, 0};
    bool detectando[5] = {false, false, false, false, false};
};

#endif
