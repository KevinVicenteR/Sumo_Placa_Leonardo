// Modo3Lado.h — Modo 3 de Rumania (DIP1 off, DIP2 ON): de lado, rival a la derecha o a la izquierda
// Colocado de lado al rival. Elige el lado: el del sensor lateral o de 45° que
// lo vea o, si ninguno lo ve, el de DIP3 (off = derecha, ON = izquierda). Gira
// pivotando sobre la rueda de ese lado (esa rueda parada, la otra adelante)
// hasta encararlo; si un sensor lo ve por el otro lado, cambia de lado. Si tras
// Modo3TiempoGiro no lo ha encontrado, sigue barriendo más despacio. Cuando el
// frontal lo ve, ataca a fondo.
// Simulador (RU3, X3 de 500 g, 8.712 combates, todos los rivales):
// gana 52,62 % / cae 22,42 %; propias 0; solo 0/792.
#ifndef MODO3_LADO_H
#define MODO3_LADO_H

#include "movimiento/rumania/PasoModo.h"

// --- Velocidades (PWM 0-255) ---
constexpr int Modo3VelocidadGiro = 200;          // rueda exterior del giro hacia el rival
constexpr int Modo3VelocidadBarrido = 120;       // si no lo encuentra tras el giro
// --- Tiempos (ms) ---
// Maniobra obligatoria al empezar (gira hacia DIP3 sin mirar los sensores)
constexpr unsigned long Modo3TiempoMinimo = 0;
constexpr unsigned long Modo3TiempoGiro = 500;   // fase inicial de giro; después, barrido
constexpr unsigned long Modo3TiempoMax = 1500;   // duración máxima del modo

class Modo3Lado {
public:
    ResultadoModo paso(const PasoModo& p);

private:
    int sentido = 0;   // 1 = gira a la derecha, -1 = a la izquierda (0 = sin decidir)
};

#endif
