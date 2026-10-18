// ParametrosGiroLateral.h — Giro hacia un rival visto por un sensor lateral
// Velocidades en PWM (0-255) y tiempos en ms. Lo incluye Parametros.h: los
// scripts de simulacion/ cambian estos valores por su nombre (--param).
#ifndef PARAMETROS_GIRO_LATERAL_H
#define PARAMETROS_GIRO_LATERAL_H

// true = la rueda de ese lado se frena y la otra gira a VelocidadRuedaPivote;
// false = giro en el sitio a VelocidadPivoteLateral.
constexpr bool GiroLateralEnRueda = false;
constexpr int VelocidadRuedaPivote = 180;
constexpr int VelocidadPivoteLateral = 220;
// Giro hacia un rival visto de lado: sigue hasta verlo de frente o este tiempo.
constexpr unsigned long TiempoMaxGiroLateral = 400;
// Una detección de frente no termina el giro antes de este tiempo.
constexpr unsigned long TiempoMinimoGiroLateral = 60;

#endif
