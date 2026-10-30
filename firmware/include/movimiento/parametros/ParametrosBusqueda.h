// ParametrosBusqueda.h — Búsqueda del rival (sin verlo con ningún sensor)
// Velocidades en PWM (0-255) y tiempos en ms. Lo incluye Parametros.h: los
// scripts de simulacion/ cambian estos valores por su nombre (--param).
#ifndef PARAMETROS_BUSQUEDA_H
#define PARAMETROS_BUSQUEDA_H

// Velocidad de avance y de giro de la búsqueda. X3 (400 rpm, 500 g): a 80/75
// iba casi parado en el robot real; 140/110 no cambia caídas en el simulador
// (100 716 combates: 80,6 % / 11,4 %, 1 propia, 1 sin rival, igual que 80/75).
constexpr int VelocidadAvance = 140;
constexpr int VelocidadGiroBusqueda = 110;

// Búsqueda: 0 = avance y giro en el sitio; 1 = arcos suaves que cambian de
// lado cada TiempoArcoBusqueda (rueda interior a PorcentajeArcoBusqueda).
constexpr int PatronBusqueda = 1;
constexpr int PorcentajeArcoBusqueda = 50;
// 1400 ms: barrido más lento que con 900 (la velocidad no se puede bajar: la
// rueda interior ya va a la zona muerta de los motores)
constexpr unsigned long TiempoArcoBusqueda = 1400;
// La búsqueda arranca acelerando poco a poco durante este tiempo.
constexpr unsigned long TiempoArranqueBusqueda = 400;

// --- Búsqueda en el sitio (PatronBusqueda = 0) ---
// Ciclo: avanza, pausa, gira, pausa.
constexpr unsigned long TiempoAvanceBusqueda = 1500;
constexpr unsigned long TiempoGiroBusqueda = 150;
constexpr unsigned long TiempoPausaBusqueda = 100;
// Avance en pulsos: TiempoPulsoAvance empujando y TiempoPausaPulso parado
// (0 = avance continuo).
constexpr unsigned long TiempoPulsoAvance = 0;
constexpr unsigned long TiempoPausaPulso = 40;
// Zigzag durante el avance: arcos alternos de TiempoArcoZigzag con la rueda
// interior a PorcentajeArcoZigzag de la exterior (100 = recto).
constexpr int PorcentajeArcoZigzag = 100;
constexpr unsigned long TiempoArcoZigzag = 0;

#endif
