#ifndef COMANDOS_CONTROL_IR_H
#define COMANDOS_CONTROL_IR_H

#include <stdint.h>

struct CodigoControlIR {
    uint8_t protocolo;
    uint16_t direccion;
    uint16_t comando;
    uint32_t hash;

    constexpr CodigoControlIR(uint8_t protocolo = 0, uint16_t direccion = 0,
                             uint16_t comando = 0, uint32_t hash = 0)
        : protocolo(protocolo), direccion(direccion), comando(comando), hash(hash) {}
};

enum class AccionControlIR : uint8_t {
    Ninguna,
    Iniciar,
    Detener,
    Reiniciar
};

constexpr bool mismoCodigoIR(const CodigoControlIR& a, const CodigoControlIR& b) {
    return a.protocolo == b.protocolo &&
           a.direccion == b.direccion &&
           a.comando == b.comando &&
           a.hash == b.hash;
}

constexpr AccionControlIR buscarAccionIR(const CodigoControlIR& recibido,
                                         const CodigoControlIR& iniciar,
                                         const CodigoControlIR& detener,
                                         const CodigoControlIR& reiniciar) {
    return mismoCodigoIR(recibido, iniciar) ? AccionControlIR::Iniciar :
           mismoCodigoIR(recibido, detener) ? AccionControlIR::Detener :
           mismoCodigoIR(recibido, reiniciar) ? AccionControlIR::Reiniciar :
           AccionControlIR::Ninguna;
}

#endif
