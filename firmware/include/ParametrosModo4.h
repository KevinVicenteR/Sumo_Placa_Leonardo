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
// pesos_neuronales_rutina3v2: los modos 1-3 se comportan como en la rama
// secihti (estrategia clásica todo el combate, giros en el sitio); las redes,
// el control por red y el pivote solo actúan en el modo 4.
constexpr bool MejorasEnModos1a3 = false;
constexpr bool UsarRedLogicaModo1 = MejorasEnModos1a3;
constexpr bool UsarRedLogicaModo3 = MejorasEnModos1a3;
constexpr bool UsarRedControlModo1 = MejorasEnModos1a3;
// Modo 3: después del arranque va de frente contra el rival: la red de lógica no
// puede esquivar (si lo intenta, decide la estrategia clásica) y la red de
// control solo ajusta las ruedas mientras ataca. Sin la red de control el modo 3
// ganaba menos y se caía más (25 200 combates: 41,1/40,2 % frente a 50,1/31,4 %).
constexpr bool UsarRedControlModo3 = MejorasEnModos1a3;
constexpr bool EsquivarModo3 = false;
// Rumania (RoboChallenge 4.1): cada robot detrás de su línea de salida y dentro
// de su franja de 10 cm; con 10 x 10 cm solo caben colocaciones rectas. Cada
// modo es una táctica rápida para una colocación: modo 1 de frente (ataque),
// modo 2 de frente (espera), modo 3 de lado, modo 4 de espaldas (DIP3 apagado =
// rival / giro a la derecha). En cualquier fase, si el sensor frontal ve al
// rival, ataque a fondo (VelocidadEmpuje).
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
// Elegidas con el simulador (colocación estricta del reglamento, todos los
// rivales y factores; % de victorias / % de caídas):
//   modo 1 arco         54,8 / 24,9 (rutina de siempre 41,0 / 39,5)
//   modo 2 contragolpe  54,3 / 27,3 (rutina de siempre 48,8 / 28,8)
//   modo 3 pivote       46,3 / 28,7 (rutina de siempre 45,5 / 34,3)
//   modo 4 contragolpe: con la red jerárquica, en RU4 (5 280 combates sin el
//     rival de 500 g) 59,3 / 23,2 y 0/400 caídas en ideal, frente a henka
//     58,0 / 24,1 y 12/400 (simulacion/barrido_modo4.py)
// pesos_neuronales_rutina3v2: rounds normales (no Rumania). Los modos 1-3 usan
// su rutina de inicio de siempre; el modo 4 (salida libre) el arco 斜め / 回り込み,
// como en pesos_neuronales_rutina3.
constexpr int TacticaModo1 = 0;
constexpr int TacticaModo2 = 0;
constexpr int TacticaModo3 = 0;
constexpr int TacticaModo4 = TacticaArco;
// Alguno de los modos usa esta táctica (las que no se usan no ocupan Flash)
constexpr bool usaTactica(int t) {
    return TacticaModo1 == t || TacticaModo2 == t || TacticaModo3 == t || TacticaModo4 == t;
}
constexpr int tacticaDeModo(int modo) {
    return modo == 1 ? TacticaModo1 : modo == 2 ? TacticaModo2 : modo == 3 ? TacticaModo3
         : modo == 4 ? TacticaModo4 : 0;
}
constexpr int VelocidadTactica = 255;
constexpr int VelocidadGiroTactica = 150;
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
