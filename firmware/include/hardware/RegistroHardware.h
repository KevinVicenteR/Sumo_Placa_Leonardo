// RegistroHardware.h — Sentido de los motores y pines de piso, anotados en la EEPROM
// Solo registro: el firmware sigue usando Pines.h y ParametrosMotores.h. El de
// combate lo anota al encender y el de diagnóstico lo imprime al empezar, para
// saber después cómo estaba configurado el robot (la EEPROM no se borra al
// cargar otro programa). Solo en el Arduino.
#ifndef REGISTRO_HARDWARE_H
#define REGISTRO_HARDWARE_H

#if defined(__AVR__)
#include <Arduino.h>
#include <EEPROM.h>
#include "Pines.h"
#include "Parametros.h"

// Bytes 1019..1022 (CajaNegra usa el principio y 1023 es el aviso de reinicio):
// marca, sentido (bit 0 = InvertirMotorIzq, bit 1 = InvertirMotorDer), pin de
// piso izquierdo y pin de piso derecho
constexpr int DireccionRegistroHardware = 1019;
constexpr uint8_t MarcaRegistroHardware = 0x5E;

inline void guardarRegistroHardware() {
    // update solo escribe los bytes que cambian (no gasta la EEPROM en cada encendido)
    EEPROM.update(DireccionRegistroHardware + 1, (InvertirMotorIzq ? 1 : 0) | (InvertirMotorDer ? 2 : 0));
    EEPROM.update(DireccionRegistroHardware + 2, S_PISO_IZQ);
    EEPROM.update(DireccionRegistroHardware + 3, S_PISO_DER);
    EEPROM.update(DireccionRegistroHardware, MarcaRegistroHardware);
}
#endif

#endif
