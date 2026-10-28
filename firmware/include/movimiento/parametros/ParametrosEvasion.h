// ParametrosEvasion.h — Evasión del borde (al ver la línea blanca)
// Velocidades en PWM (0-255) y tiempos en ms. Lo incluye Parametros.h: los
// scripts de simulacion/ cambian estos valores por su nombre (--param).
#ifndef PARAMETROS_EVASION_H
#define PARAMETROS_EVASION_H

// Velocidades del retroceso y del giro.
constexpr int VelocidadRetroceso = 255;
constexpr int VelocidadGiroEvasion = 75;

// true = la evasión del borde también pivota (hacia atrás); false = gira en el sitio
constexpr bool PivoteEnEvasion = false;

// Solo la usan los tests y la simulación como referencia.
constexpr unsigned long TiempoRetroceso = 180;
// Retroceso mínimo con los dos sensores en blanco / con uno solo.
constexpr unsigned long TiempoRetrocesoAmbos = 200;
constexpr unsigned long TiempoRetrocesoUnSensor = 90;
constexpr int VelocidadRetrocesoUnSensor = 118;
// Tiempo viendo negro seguido antes de dejar de retroceder.
constexpr unsigned long TiempoSeparacionBorde = 90;
constexpr unsigned long TiempoSeparacionUnSensor = 50;
// Tope de toda la maniobra (no hay sensores traseros que avisen).
constexpr unsigned long TiempoMaximoRecuperacionBorde = 700;
// true = si algún sensor ve al rival durante la evasión, corta la evasión y va
// contra él. Solo después de separarse de la línea y frenar (mientras gira o en
// la pausa final), nunca retrocediendo, y como mucho una vez cada
// EsperaEntreCortesEvasion ms (si no, se queda atrapado en el borde cortando
// evasiones). La detección debe durar TiempoConfirmarRivalEnEvasion ms seguidos.
constexpr bool CortarEvasionAlVerRival = true;
constexpr unsigned long TiempoConfirmarRivalEnEvasion = 20;
constexpr unsigned long EsperaEntreCortesEvasion = 2000;
// Si sigue viendo la línea parado más de este tiempo (por ejemplo, al agotar
// TiempoMaximoRecuperacionBorde), vuelve a retroceder despacio durante
// TiempoReintentoRetroceso: nunca se queda quieto sobre la línea. No gira: con
// las ruedas atrás, girar junto al borde barre la pala hacia fuera y se sale.
constexpr unsigned long TiempoMaxParadoEnLinea = 300;
constexpr unsigned long TiempoReintentoRetroceso = 150;
// Pausa con motores parados entre retroceso y giro, y después del giro.
constexpr unsigned long TiempoFrenado = 80;
constexpr unsigned long TiempoAsentamientoEvasion = 100;
// Duración del giro con un sensor / con los dos sensores en blanco.
constexpr unsigned long TiempoGiroEvasion = 240;
constexpr unsigned long TiempoGiroEvasionAmbos = 300;
// Al ver la línea, primero retrocede y después gira. true = si no venía
// avanzando (velocidad media estimada < AvanceMinimoParaRetroceder, en PWM)
// gira en el sitio sin retroceder. Desactivado: con las ruedas atrás el robot
// pivota sobre el eje trasero y, en el robot real, girando junto al borde se sale.
constexpr bool SalirGirandoSiNoAvanza = false;
constexpr int AvanceMinimoParaRetroceder = 50;
// Retroceso extra (ms) si llega a la línea a velocidad máxima (proporcional a la
// velocidad). Con poco agarre el robot sigue deslizando hacia fuera al frenar y,
// con el morro fuera del dohyo, los sensores de piso pueden leer "negro".
constexpr unsigned long RetrocesoExtraAVelocidadMaxima = 300;
// Tras evadir con un solo sensor, sale despacio durante este tiempo.
constexpr unsigned long TiempoSalidaSuaveUnSensor = 500;
constexpr int VelocidadSalidaUnSensor = 90;

// Propuestas de docs/caidas_propias_rumania.md (false = comportamiento anterior):
// salida suave también tras cortar la evasión o tras verla con los dos sensores
constexpr bool SalidaSuaveSiempre = false;
// las reentradas en la línea comparten un solo tope de TiempoReintentoRetroceso
// de retroceso por evasión, sin renovarlo en cada reentrada
constexpr bool PresupuestoRetroceso = false;
// tras ver la línea, si el estimador deja de ser fiable conserva el último
// límite de avance (como mucho LimiteTrasLinea) en vez de quitarlo. Con 100: 0
// salidas sin rival pero -7 puntos de victorias (simulador, 50 820 combates)
constexpr bool ProtegerTrasLinea = false;
constexpr int LimiteTrasLinea = 100;
// Cuánto dura la protección tras la última línea vista (0 = siempre)
constexpr unsigned long ProtegerTrasLineaMs = 0;

#endif
