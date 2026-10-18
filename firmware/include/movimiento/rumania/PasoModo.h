// PasoModo.h — Lo común a los cuatro modos de Rumania (RoboChallenge 4.1)
// Cada robot se coloca detrás de su línea de salida, dentro de su franja de
// 10 cm; con 10 x 10 cm solo caben colocaciones rectas. Los DIP eligen el modo:
//   DIP1 DIP2  modo
//   off  off   1: de frente, defensivo   (Modo1Defensivo.h)
//   ON   off   2: de frente, contragolpe (Modo2Contragolpe.h)
//   off  ON    3: de lado, predictivo    (Modo3Predictivo.h)
//   ON   ON    4: de espaldas, agresivo  (Modo4Agresivo.h)
//   DIP3: lado del rival / del giro (off = derecha, ON = izquierda)
// Cada archivo de modo tiene sus velocidades, sus tiempos y su lógica. Los
// modos solo dirigen el arranque: en cuanto el sensor frontal ve al rival
// (confirmado) o se agota su tiempo, el combate sigue con la estrategia normal.
#ifndef PASO_MODO_H
#define PASO_MODO_H

#include "movimiento/MandoMotores.h"
#include "estrategia/DecisionMovimiento.h"

// true = los modos 1-4 son los de Rumania; false = las rutinas de inicio de
// siempre (rondas espalda, lado, enfrentados y salida libre).
constexpr bool UsarModosRumania = true;

// Lo que un modo necesita saber en cada ciclo
struct PasoModo {
    const DecisionMovimiento& decision;
    MandoMotores& mando;
    IMotor& motor;
    unsigned long ahora;
    unsigned long t;     // ms desde que empezó el modo
    int lado;            // DIP3: 1 = derecha, -1 = izquierda
    int visto;           // sensor de 45° o lateral que lo ve: 1 der, -1 izq, 0 ninguno
    bool deFrente;       // el frontal lo ve (confirmado)
};

// Qué hace el modo tras este ciclo
enum class ResultadoModo : unsigned char {
    Sigue,      // sigue el modo (ya ha mandado los motores)
    Ataca,      // termina con ataque a fondo (ControlAtaque, VelocidadEmpuje)
    Termina     // termina sin más: sigue la estrategia normal
};

#endif
