#ifndef RED_CONTROL_H
#define RED_CONTROL_H

#include <stdint.h>

namespace redcontrol {
constexpr int NumEntradasRed = 7;

void corregir(const int8_t x[NumEntradasRed], int& avance, int& giro);

#if !defined(__AVR__)
extern int8_t Parametros[];
int numParametros();
#endif
}

#endif
