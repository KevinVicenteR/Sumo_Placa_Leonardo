// ParametrosEstrategia.h — Estrategia: política aprendida y redes neuronales
// Lo incluye Parametros.h: los scripts de simulacion/ cambian estos valores
// por su nombre (--param). Velocidades en PWM (0-255), tiempos en ms y
// distancias en metros, salvo que se indique otra cosa.
#ifndef PARAMETROS_ESTRATEGIA_H
#define PARAMETROS_ESTRATEGIA_H

// --- Política aprendida ---
// true = usa la política aprendida en el simulador (TablaPolitica.h) en vez de
// las reglas escritas a mano.
constexpr bool UsarPoliticaAprendida = false;

// --- Redes neuronales (lógica: qué hacer; control: cómo mover las ruedas) ---
// Pesos en estrategia/TablaRedLogica.h y movimiento/TablaRedControl.h.
constexpr bool UsarRedLogica = true;
constexpr bool UsarRedControl = true;
// Selección según pruebas por modo (25 200 combates a 77 cm). Las redes actúan
// en los modos 1 y 3 solo después de su rutina de inicio; el modo 3 va de frente.
// Sumo X3 sobre rutina3v2: los modos 1-3 se comportan como en la rama
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
constexpr bool MejorarRecuperacionModo3 = false;

// Entradas de la red de lógica: recuerdo del rival y contacto de frente (ms)
constexpr unsigned long RecuerdoRivalMs = 300;
constexpr unsigned long ContactoFrontalMs = 200;

// Esquiva decidida por la red: duración mínima, rearme y velocidades de las ruedas
constexpr unsigned long TiempoMinimoEsquivaRed = 120;
constexpr unsigned long TiempoRearmeEsquivaRed = 400;
constexpr int VelocidadEsquivaInterior = 60;
constexpr int VelocidadEsquivaExterior = 255;

#endif
