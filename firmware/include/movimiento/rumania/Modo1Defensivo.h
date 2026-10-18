// Modo1Defensivo.h — Modo 1 de Rumania (DIP1 off, DIP2 off): de frente, defensivo (受け)
// Quieto encarándolo: si un sensor de 45° o lateral lo ve, gira hacia él. Cuando
// el frontal lo ve, lo recibe parado con la pala Modo1TiempoRecibir ms y después
// empuja a fondo. Si no aparece en Modo1TiempoQuieto, avanza despacio a buscarlo.
// Simulador (RU1, sin el rival de 500 g): gana 66,0 % / cae 18,7 % (arco 63,4 / 18,0).
#ifndef MODO1_DEFENSIVO_H
#define MODO1_DEFENSIVO_H

#include "movimiento/rumania/PasoModo.h"

// --- Velocidades (PWM 0-255) ---
constexpr int Modo1VelocidadGiro = 200;          // giro hacia el sensor que lo ve
constexpr int Modo1VelocidadAvanceEspera = 50;  // avance lento si no aparece
// --- Tiempos (ms) ---
// Maniobra obligatoria al empezar (quieto sin mirar los sensores). 0: en el
// simulador, 1000 ms subía las caídas del modo 1 del 25 % al 43 % (el rival
// embiste y lo saca mientras está ciego)
constexpr unsigned long Modo1TiempoMinimo = 0;
constexpr unsigned long Modo1TiempoQuieto = 1000;    // quieto antes de avanzar
constexpr unsigned long Modo1TiempoRecibir = 200;   // parado con la pala al verlo de frente
constexpr unsigned long Modo1TiempoMax = 9000;      // duración máxima del modo

class Modo1Defensivo {
public:
    ResultadoModo paso(const PasoModo& p);

private:
    bool recibiendo = false;
    unsigned long inicioRecibir = 0;
};

#endif
