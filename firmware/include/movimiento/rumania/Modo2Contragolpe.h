// Modo2Contragolpe.h — Modo 2 de Rumania (DIP1 ON, DIP2 off): de frente, contragolpe
// Quieto hasta verlo: si un sensor de 45° o lateral lo ve, gira hacia él; cuando
// el frontal lo ve, ataca a fondo al instante. Si no aparece en
// Modo2TiempoQuieto, avanza despacio a buscarlo.
// Simulador (RU2, X3 de 500 g, 8.712 combates, todos los rivales):
// gana 63,84 % / cae 14,94 %; propias 0; solo 0/792.
#ifndef MODO2_CONTRAGOLPE_H
#define MODO2_CONTRAGOLPE_H

#include "movimiento/rumania/PasoModo.h"

// --- Velocidades (PWM 0-255) ---
constexpr int Modo2VelocidadGiro = 150;          // giro hacia el sensor que lo ve
constexpr int Modo2VelocidadAvanceEspera = 100;  // avance lento si no aparece
// --- Tiempos (ms) ---
// Tramo inicial quieto sin atender al rival; la línea siempre lo interrumpe.
// Reducido para reaccionar antes en el X3 de 500 g.
constexpr unsigned long Modo2TiempoMinimo = 50;
constexpr unsigned long Modo2TiempoQuieto = 1500;    // quieto antes de avanzar
constexpr unsigned long Modo2TiempoMax = 6000;      // duración máxima del modo

class Modo2Contragolpe {
public:
    ResultadoModo paso(const PasoModo& p);
};

#endif
