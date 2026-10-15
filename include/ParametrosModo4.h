// Parámetros de la ruta mejorada: desde el inicio del modo 4 y, en los
// modos 1–3, únicamente después de sus maniobras iniciales protegidas.
#ifndef PARAMETROS_MODO4_H
#define PARAMETROS_MODO4_H
#include "Parametros.h"
constexpr bool UsarRedLogica = true;
constexpr bool UsarRedControl = true;
// El PID ensayado perdió rapidez y victorias; no se activa en este perfil.
constexpr bool UsarPIDRed = false;
// Selección según pruebas por modo. El modo 3 conserva su comportamiento.
constexpr bool UsarRedLogicaModo1 = false;
constexpr bool UsarRedLogicaModo3 = false;
constexpr bool UsarRedControlModo1 = false;
constexpr bool UsarRedControlModo3 = false;
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
