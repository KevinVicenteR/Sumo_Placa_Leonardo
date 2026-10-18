// ModoDefensivo.h — Estilo defensivo (受け): quieto encarándolo, lo recibe con la pala y empuja (en qué modo se usa: PasoModo.h)
// Quieto encarándolo: si un sensor de 45° o lateral lo ve, gira hacia él. Cuando
// el frontal lo ve, lo recibe parado con la pala DefensivoTiempoRecibir ms y después
// empuja a fondo. Si no aparece en DefensivoTiempoQuieto, avanza despacio a buscarlo.
// Simulador (RU1, sin el rival de 500 g): gana 66,0 % / cae 18,7 % (arco 63,4 / 18,0).
#ifndef MODO_DEFENSIVO_H
#define MODO_DEFENSIVO_H

#include "movimiento/modos/PasoModo.h"

// --- Velocidades (PWM 0-255) ---
constexpr int DefensivoVelocidadGiro = 200;          // giro hacia el sensor que lo ve
constexpr int DefensivoVelocidadAvanceEspera = 50;  // avance lento si no aparece
// --- Tiempos (ms) ---
// Maniobra obligatoria al empezar (quieto sin mirar los sensores). 0: en el
// simulador, 1000 ms subía las caídas del modo 1 del 25 % al 43 % (el rival
// embiste y lo saca mientras está ciego)
constexpr unsigned long DefensivoTiempoMinimo = 0;
constexpr unsigned long DefensivoTiempoQuieto = 3000;    // quieto antes de avanzar
constexpr unsigned long DefensivoTiempoRecibir = 200;   // parado con la pala al verlo de frente
constexpr unsigned long DefensivoTiempoMax = 9000;      // duración máxima del modo

class ModoDefensivo {
public:
    ResultadoModo paso(const PasoModo& p);

private:
    bool recibiendo = false;
    unsigned long inicioRecibir = 0;
};

#endif
