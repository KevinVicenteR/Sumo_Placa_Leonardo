// Modo4Agresivo.h — Modo 4 de Rumania (DIP1 ON, DIP2 ON): de espaldas, agresivo
// Avanza recto sin verlo; si un sensor de 45° o lateral lo ve, gira hacia él.
// Al detectar el frontal, deja de girar y avanza mientras se confirma la
// detección; confirmado, ataca a fondo. La línea siempre cancela el modo.
// Simulador (RU4, X3 de 500 g, 28.776 combates, todos los rivales): gana 46,7 % /
// cae 31,3 % (sin estabilizar 44,9 / 30,9). Con 225 en la confirmación ganaba
// 46,1 / 30,8 pero con 4 caídas propias (peor caso, rival quieto o tornado).
#ifndef MODO4_AGRESIVO_H
#define MODO4_AGRESIVO_H

#include "movimiento/rumania/PasoModo.h"

// --- Velocidades (PWM 0-255) ---
// Recto sin verlo: no a fondo, porque a 255 el robot real no llega a frenar al
// ver la línea y se sale solo.
constexpr int Modo4VelocidadAvance = 150;
constexpr int Modo4VelocidadGiro = 255;          // giro hacia el sensor que lo ve
// Deja de girar al detectar el frontal, antes de terminar su confirmación.
// PWM de avance durante esa confirmación (0 = parado).
constexpr bool Modo4EstabilizarFrontal = true;
constexpr int Modo4VelocidadConfirmacion = 150;
// --- Tiempos (ms) ---
// Sin maniobra ciega: atiende los sensores desde START.
constexpr unsigned long Modo4TiempoMinimo = 0;
constexpr unsigned long Modo4TiempoMax = 1500;   // duración máxima del modo

class Modo4Agresivo {
public:
    ResultadoModo paso(const PasoModo& p);
};

#endif
