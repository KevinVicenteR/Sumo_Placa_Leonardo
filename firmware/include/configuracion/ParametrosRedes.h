#ifndef PARAMETROS_MODO4_H
#define PARAMETROS_MODO4_H
#include "configuracion/Parametros.h"
constexpr bool UsarRedLogica = true;
constexpr bool UsarRedControl = true;
constexpr bool UsarPIDRed = false;
constexpr bool UsarRedLogicaModo1 = true;
constexpr bool UsarRedLogicaModo3 = true;
constexpr bool UsarRedControlModo1 = true;
constexpr bool UsarRedControlModo3 = true;
constexpr bool EsquivarModo3 = false;
constexpr unsigned long TiempoMaxAtaqueLibre = 150;
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
