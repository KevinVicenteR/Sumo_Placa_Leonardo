// Modo3Predictivo.h — Modo 3 de Rumania (DIP1 off, DIP2 ON): de lado, predictivo
// Si un sensor de 45° o lateral lo ve, avanza en arco hacia ese lado para
// cortarle el paso donde va a estar; si lo pierde, barre hacia donde lo vio por
// última vez (al empezar, hacia DIP3). Cuando el frontal lo ve, ataca a fondo.
// Simulador (RU3, sin el rival de 500 g): gana 57,3 % / cae 21,6 % (pivote 54,2 / 23,9).
#ifndef MODO3_PREDICTIVO_H
#define MODO3_PREDICTIVO_H

#include "movimiento/rumania/PasoModo.h"

// --- Velocidades (PWM 0-255) ---
// Arco de intercepción: rueda exterior e interior. Sin ver al rival no va a
// fondo: a 255 el robot real no llega a frenar al ver la línea y se sale solo.
constexpr int Modo3VelocidadArco = 150;
constexpr int Modo3VelocidadArcoInterior = 60;
constexpr int Modo3VelocidadBarrido = 100;       // barrido sin verlo
// --- Tiempos (ms) ---
constexpr unsigned long Modo3TiempoMax = 1500;   // duración máxima del modo

class Modo3Predictivo {
public:
    ResultadoModo paso(const PasoModo& p);

private:
    int ultimoVisto = 0;   // último lado en que lo vio un sensor de 45° o lateral
};

#endif
