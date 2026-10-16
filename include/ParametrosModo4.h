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
// Rumania: cada uno coloca su robot donde quiere y cada modo es una táctica
// rápida para una colocación (modo 1 de frente, 2 a 45°, 3 de lado, 4 de
// espaldas; DIP3 = lado del rival / del giro). En cualquier fase, si el sensor
// frontal ve al rival, ataque a fondo (VelocidadEmpuje).
//   0 = la rutina de inicio de siempre de ese modo
//   1 = carga recta a fondo                 2 = contragolpe: quieto hasta verlo
//   3 = pivote sobre una rueda hacia DIP3   4 = giro rápido en el sitio
//   5 = 斜め / 回り込み: arco a fondo hacia DIP3 y giro hacia dentro
//   6 = 受け (henka): atrás en diagonal y giro
//   7 = espera y esquiva: si lo ve pronto (viene embistiendo) se aparta
//       pivotando hacia DIP3 y gira para pillarlo de lado; si no, contragolpe
// Con un sensor de 45° o lateral viéndolo, gira rápido hacia ese lado.
constexpr int TacticaCarga = 1, TacticaContragolpe = 2, TacticaPivote = 3, TacticaGiro = 4;
constexpr int TacticaArco = 5, TacticaHenka = 6, TacticaEsperaEsquiva = 7;
// Elegidas con simulacion/barrido_tacticas.py (X3, 30 576 combates por táctica):
// la que más gana sin caerse más sola ni ganar menos en ideal que la rutina 0.
//   modo 1: 斜め 42,8 % gana / 27,7 % cae / 0 propias (rutina 0: 34,2 / 41,4 / 91)
//   modos 2, 3 y 4: ninguna mejora a la rutina de siempre con ese criterio
constexpr int TacticaModo1 = TacticaArco;
constexpr int TacticaModo2 = 0;
constexpr int TacticaModo3 = 0;
constexpr int TacticaModo4 = 0;
constexpr int tacticaDeModo(int modo) {
    return modo == 1 ? TacticaModo1 : modo == 2 ? TacticaModo2 : modo == 3 ? TacticaModo3
         : modo == 4 ? TacticaModo4 : 0;
}
constexpr int VelocidadTactica = 255;
constexpr int VelocidadGiroTactica = 200;
constexpr int VelocidadInteriorArcoLibre = 110;
constexpr unsigned long TiempoRetrocesoLibre = 180;
constexpr unsigned long TiempoArcoLibre = 200;
constexpr unsigned long TiempoMaxTactica = 1500;
constexpr unsigned long TiempoMaxEsperaTactica = 3000;
constexpr unsigned long VentanaRivalRapidoTactica = 600;
constexpr unsigned long TiempoEsquivaTactica = 120;
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
