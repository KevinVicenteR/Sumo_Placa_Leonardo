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
// Modo 4 (salida libre): maniobra de inicio, todas a máxima velocidad. En
// cualquier fase, si el frontal ve al rival, ataque a fondo (VelocidadEmpuje).
//   0 = giro en el sitio a VelocidadGiroRound4 hasta verlo (la de siempre)
//   1 = 受け (henka) y contraataque: retrocede en diagonal con la rueda del
//       lado de DIP3 (TiempoRetrocesoLibre) para salir de la línea de su
//       embestida y gira rápido hasta encararlo
//   2 = 斜め / 回り込み: arco a fondo hacia DIP3 (TiempoArcoLibre) y giro
//       rápido hacia dentro para atacarlo de lado
//   3 = tornado: giro rápido en el sitio hasta verlo y carga a fondo
// Elegida con el simulador (modo 4 colocado como en los rounds, 20 160 combates):
// 2 gana 46,4 % y se cae 26,9 % (0: 38,8 % / 32,0 %). Colocado al azar (a veces
// mirando al borde) es peor que 0: colócalo de cara al rival, con DIP3 hacia
// el lado por el que quieras rodearlo.
constexpr int EstrategiaLibre = 2;
constexpr int VelocidadManiobraLibre = 255;
constexpr int VelocidadInteriorArcoLibre = 110;
constexpr int VelocidadGiroLibre = 200;
constexpr unsigned long TiempoRetrocesoLibre = 180;
constexpr unsigned long TiempoArcoLibre = 200;
constexpr unsigned long TiempoMaxManiobraLibre = 1500;
// Ataque a fondo sin llegar a empujar durante este tiempo: pasa al ataque normal
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
