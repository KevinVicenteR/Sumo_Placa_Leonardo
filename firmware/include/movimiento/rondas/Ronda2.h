// Ronda2.h — Modo 2 (DIP1 ON, DIP2 off): lado a lado
// El lado lo decide el primer sensor lateral o de 45° que vea al rival (si no lo
// ve ninguno, DIP3); una vez decidido no cambia. Gira hacia ese lado (pivotando
// sobre la rueda de ese lado o en el sitio) hasta verlo de frente; al verlo
// frena el giro y lo embiste (VelocidadEmbestida* en ParametrosAtaque.h).
#ifndef RONDA2_H
#define RONDA2_H

#include "movimiento/rondas/PasoRonda.h"
#include "movimiento/Confirmacion.h"

// --- Velocidades (PWM 0-255) ---
// true = pivota sobre la rueda del lado del rival (esa rueda parada y la otra
// empujando), avanzando a la vez contra él; false = gira en el sitio.
constexpr bool Ronda2Pivote = false; // X3: giro en el sitio.
constexpr int Ronda2VelocidadPivote = 180;        // rueda que empuja en el pivote
// Giro en el sitio (con Ronda2Pivote = false): el rival está al lado, girar
// despacio le deja golpear
constexpr int Ronda2VelocidadGiro = 150;
constexpr int Ronda2VelocidadEsquiva = 160;       // avance recto antes de girar
// --- Tiempos (ms) ---
constexpr unsigned long Ronda2TiempoEsquiva = 0;       // avanza recto antes de girar (0 = no)
constexpr unsigned long Ronda2TiempoMinimoGiro = 60;   // una detección no termina el giro antes
constexpr unsigned long Ronda2TiempoMaxGiro = 500;     // duración máxima del giro

class Ronda2 {
public:
    ResultadoRonda paso(const PasoRonda& p);

private:
    void decidirLado(const PasoRonda& p);
    Confirmacion rivalVisto;
    bool iniciada = false;
    int lado = 1;
    bool ladoDecidido = false;
    bool frenando = false;
    unsigned long inicioFreno = 0;
    Freno freno;
};

#endif
