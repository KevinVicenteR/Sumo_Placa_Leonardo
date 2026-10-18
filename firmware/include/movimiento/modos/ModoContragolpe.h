// ModoContragolpe.h — Estilo contragolpe: quieto hasta verlo y ataca a fondo (en qué modo se usa: PasoModo.h)
// Quieto hasta verlo: si un sensor de 45° o lateral lo ve, gira hacia él; cuando
// el frontal lo ve, ataca a fondo al instante. Si no aparece en
// ContragolpeTiempoQuieto, avanza despacio a buscarlo.
// Simulador (RU2, sin el rival de 500 g): gana 65,5 % / cae 18,2 %.
// En pesos_neuronales_rutina3v2 es el modo 3 (enfrentados, lejos del rival):
// en R3 gana 53,2 % / cae 35,0 % (el avance lento de antes: 65,0 / 24,1; sin
// la espera inicial, ContragolpeTiempoMinimo y ContragolpeTiempoQuieto a 0: 62,4 / 25,7).
#ifndef MODO_CONTRAGOLPE_H
#define MODO_CONTRAGOLPE_H

#include "movimiento/modos/PasoModo.h"

// --- Velocidades (PWM 0-255) ---
constexpr int ContragolpeVelocidadGiro = 150;          // giro hacia el sensor que lo ve
constexpr int ContragolpeVelocidadAvanceEspera = 100;  // avance lento si no aparece
// --- Tiempos (ms) ---
// Siempre hace primero su maniobra (quieto) durante este tiempo, aunque los
// sensores vean algo; solo la línea del borde la interrumpe
// (simulador, RU2: con 300 ms gana 63,2 % / cae 23,5 %; sin ella 61,4 / 23,8;
// con 800 ms 51,6 / 37,3)
constexpr unsigned long ContragolpeTiempoMinimo = 300;
constexpr unsigned long ContragolpeTiempoQuieto = 800;    // quieto antes de avanzar
constexpr unsigned long ContragolpeTiempoMax = 3000;      // duración máxima del modo

class ModoContragolpe {
public:
    ResultadoModo paso(const PasoModo& p);
};

#endif
