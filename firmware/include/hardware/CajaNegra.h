#ifndef CAJA_NEGRA_H
#define CAJA_NEGRA_H

#include <stdint.h>
#include "sensores/LecturasSensores.h"

struct RegistroCombate {
    uint16_t numero;
    uint8_t dip;
    uint8_t interrupciones;
    uint16_t negroIzq;
    uint16_t negroDer;
    uint16_t duracionDecimas;
    uint16_t evasiones;
    uint8_t maxEnemigo[5];
    uint8_t reservado;
};

class CajaNegra {
public:
    static constexpr uint8_t NumRegistros = 8;

    void empezar(uint8_t dip, int negroIzq, int negroDer, unsigned long ahora);
    void observar(const LecturasSensores& lecturas, unsigned int evasiones, unsigned long ahora);
    void anotarInterrupcion();
    void guardar();

    bool leer(uint8_t i, RegistroCombate& registro) const;
    void borrar();

private:
    struct Cabecera {
        uint16_t firma;
        uint16_t siguiente;
    };
    static constexpr uint16_t Firma = 0x534D;

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
