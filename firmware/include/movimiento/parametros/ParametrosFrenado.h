// ParametrosFrenado.h — Frenado al perder al rival y contramando de los frenos
// Velocidades en PWM (0-255) y tiempos en ms. Lo incluye Parametros.h: los
// scripts de simulacion/ cambian estos valores por su nombre (--param).
#ifndef PARAMETROS_FRENADO_H
#define PARAMETROS_FRENADO_H

// X3: parada con PWM 0 para evitar retroceder tras un choque.
constexpr bool FrenoActivo = false;

// Contramando al frenar: se limita para no dar golpes a PWM 255.
constexpr int VelocidadFreno = 140;

// Cada rueda frena según su velocidad estimada (la orden filtrada con la
// inercia TauRuedas). Por debajo de VelocidadMinimaFreno no hace falta frenar.
constexpr unsigned long TiempoFrenadoRuedas = 80;
constexpr unsigned long TauRuedas = 80;
constexpr int VelocidadMinimaFreno = 20;
// Duración máxima del frenado cuando el rival desaparece en pleno ataque.
constexpr unsigned long TiempoParoPerdida = 150;

#endif
