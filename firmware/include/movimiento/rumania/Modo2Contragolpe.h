// Modo2Contragolpe.h — Modo 2 de Rumania (DIP1 ON, DIP2 off): de frente, contragolpe
// Quieto hasta verlo: si un sensor de 45° o lateral lo ve, gira hacia él; cuando
// el frontal lo ve, ataca a fondo al instante. Si no aparece en
// Modo2TiempoQuieto, avanza despacio a buscarlo.
// Simulador (RU2, sin el rival de 500 g): gana 65,5 % / cae 18,2 %.
#ifndef MODO2_CONTRAGOLPE_H
#define MODO2_CONTRAGOLPE_H

#include "movimiento/rumania/PasoModo.h"

// --- Velocidades (PWM 0-255) ---
constexpr int Modo2VelocidadGiro = 150;          // giro hacia el sensor que lo ve
constexpr int Modo2VelocidadAvanceEspera = 100;  // avance lento si no aparece
// --- Tiempos (ms) ---
// Siempre hace primero su maniobra (quieto) durante este tiempo, aunque los
// sensores vean algo; solo la línea del borde la interrumpe
// (simulador, RU2: con 300 ms gana 63,2 % / cae 23,5 %; sin ella 61,4 / 23,8;
// con 800 ms 51,6 / 37,3)
constexpr unsigned long Modo2TiempoMinimo = 300;
constexpr unsigned long Modo2TiempoQuieto = 800;    // quieto antes de avanzar
constexpr unsigned long Modo2TiempoMax = 3000;      // duración máxima del modo

class Modo2Contragolpe {
public:
    ResultadoModo paso(const PasoModo& p);
};

#endif
