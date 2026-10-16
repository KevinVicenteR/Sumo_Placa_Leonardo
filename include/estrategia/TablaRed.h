// TablaRed.h — Cómo se guardan los pesos de las redes (TablaRedLogica.h y
// TablaRedControl.h)
// En el Leonardo van en Flash (PROGMEM) y no ocupan RAM. En el simulador y en
// los tests son variables normales para que el simulador pueda cargar otros
// pesos con --red sin recompilar (lo usa simulacion/entrenar_redes.py).
#ifndef TABLA_RED_H
#define TABLA_RED_H

#include <stdint.h>

#if defined(__AVR__)
#include <avr/pgmspace.h>
#define TABLA_RED(nombre, n) const int8_t nombre[n] PROGMEM
#define LEER_RED(tabla, i) ((int8_t)pgm_read_byte(&(tabla)[i]))
#else
#define TABLA_RED(nombre, n) int8_t nombre[n]
#define LEER_RED(tabla, i) ((tabla)[i])
#endif

#endif
