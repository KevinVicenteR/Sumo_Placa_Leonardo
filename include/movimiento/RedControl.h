// RedControl.h — Red de control: cómo mover las ruedas al atacar o esquivar
// Perceptrón con una capa oculta (TablaRedControl.h): 7 entradas (posición del
// rival y su cambio, empujando, tiempo atacando, borde delante, velocidad y
// esquiva) -> 6 neuronas ReLU -> 2 correcciones (avance y giro: la suma / 16,
// en PWM) que se suman al movimiento de siempre (PID y ControlAtaque). Con todo a cero no
// cambia nada. Solo enteros.
#ifndef RED_CONTROL_H
#define RED_CONTROL_H

#include <stdint.h>

namespace redcontrol {

constexpr int NumEntradasRed = 7;

// Entradas normalizadas a [-16,16], como las produce ControlRedModo4.
// Corrección de avance y giro (PWM, -255..255) para estas entradas
void corregir(const int8_t x[NumEntradasRed], int& avance, int& giro);

#if !defined(__AVR__)
// Pesos modificables (el simulador los carga con --red)
extern int8_t Parametros[];
int numParametros();
#endif

}

#endif
