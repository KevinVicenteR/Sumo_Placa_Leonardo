// ModoLado.h — Estilo de lado: rival a la derecha o a la izquierda, pivota hacia él y ataca (en qué modo se usa: PasoModo.h)
// Colocado de lado al rival. Elige el lado: el del sensor lateral o de 45° que
// lo vea o, si ninguno lo ve, el de DIP3 (off = derecha, ON = izquierda). Gira
// pivotando sobre la rueda de ese lado (esa rueda parada, la otra adelante)
// hasta encararlo; si un sensor lo ve por el otro lado, cambia de lado. Si tras
// LadoTiempoGiro no lo ha encontrado, sigue barriendo más despacio. Cuando el
// frontal lo ve, ataca a fondo.
// Simulador (RU3, rival a la derecha y a la izquierda, sin el de 500 g): gana
// 56,9 % / cae 22,2 % (el predictivo de antes: 57,4 / 24,9). Giro a 200: 55,6 /
// 22,2; a 255: 54,1 / 22,7.
#ifndef MODO_LADO_H
#define MODO_LADO_H

#include "movimiento/modos/PasoModo.h"

// --- Velocidades (PWM 0-255) ---
constexpr int LadoVelocidadGiro = 150;          // rueda exterior del giro hacia el rival
constexpr int LadoVelocidadBarrido = 120;       // si no lo encuentra tras el giro
// --- Tiempos (ms) ---
// Maniobra obligatoria al empezar (gira hacia DIP3 sin mirar los sensores)
constexpr unsigned long LadoTiempoMinimo = 0;
constexpr unsigned long LadoTiempoGiro = 500;   // giro rápido (unos 90°)
constexpr unsigned long LadoTiempoMax = 1500;   // duración máxima del modo

class ModoLado {
public:
    ResultadoModo paso(const PasoModo& p);

private:
    int sentido = 0;   // 1 = gira a la derecha, -1 = a la izquierda (0 = sin decidir)
};

#endif
