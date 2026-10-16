// Parámetros de la ruta mejorada: desde el inicio del modo 4 y, en los
// modos 1–3, únicamente después de sus maniobras iniciales protegidas.
#ifndef PARAMETROS_MODO4_H
#define PARAMETROS_MODO4_H
#include "Parametros.h"
constexpr bool UsarRedLogica = true;
constexpr bool UsarRedControl = true;
// El PID ensayado perdió rapidez y victorias; no se activa en este perfil.
constexpr bool UsarPIDRed = false;
// Selección según pruebas por modo (25 200 combates a 77 cm). Las redes actúan
// en los modos 1 y 3 solo después de su rutina de inicio; el modo 3 va de frente.
constexpr bool UsarRedLogicaModo1 = true;
constexpr bool UsarRedLogicaModo3 = true;
constexpr bool UsarRedControlModo1 = true;
// Modo 3: después del arranque va de frente contra el rival: la red de lógica no
// puede esquivar (si lo intenta, decide la estrategia clásica) y la red de
// control solo ajusta las ruedas mientras ataca. Sin la red de control el modo 3
// ganaba menos y se caía más (25 200 combates: 41,1/40,2 % frente a 50,1/31,4 %).
constexpr bool UsarRedControlModo3 = true;
constexpr bool EsquivarModo3 = false;
constexpr bool MejorarRecuperacionModo3 = false;
constexpr float KpPID = 0.0014f;
constexpr float KiPID = 0.0f;
constexpr float KdPID = 1.0f;
constexpr unsigned long PeriodoPID = 10;
constexpr float FiltroDerivadaPID = 0.5f;
constexpr unsigned long PausaReinicioPID = 50;
constexpr int PosicionSensor45PID = 1000;
constexpr float LimitePIDEmpujando = 0.3f;
constexpr unsigned long RecuerdoRivalMs = 300;
constexpr unsigned long ContactoFrontalMs = 200;
constexpr unsigned long TiempoMinimoEsquivaRed = 120;
constexpr unsigned long TiempoRearmeEsquivaRed = 400;
constexpr int VelocidadEsquivaInterior = 60;
constexpr int VelocidadEsquivaExterior = 255;
#endif
