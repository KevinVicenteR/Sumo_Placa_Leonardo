// ParametrosPID.h — PID de la red de control (desactivado)
// Lo incluye Parametros.h: los scripts de simulacion/ cambian estos valores
// por su nombre (--param). Velocidades en PWM (0-255), tiempos en ms y
// distancias en metros, salvo que se indique otra cosa.
#ifndef PARAMETROS_P_I_D_H
#define PARAMETROS_P_I_D_H

// El PID ensayado perdió rapidez y victorias; no se activa en este perfil.
constexpr bool UsarPIDRed = false;
constexpr float KpPID = 0.0014f;
constexpr float KiPID = 0.0f;
constexpr float KdPID = 1.0f;
constexpr unsigned long PeriodoPID = 10;
constexpr float FiltroDerivadaPID = 0.5f;
constexpr unsigned long PausaReinicioPID = 50;
constexpr int PosicionSensor45PID = 1000;
constexpr float LimitePIDEmpujando = 0.3f;

#endif
