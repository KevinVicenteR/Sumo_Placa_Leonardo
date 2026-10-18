// PasoRonda.h — Lo común a las cuatro rondas (modos elegidos con los DIP)
//   DIP1 DIP2  modo
//   off  off   1: espalda con espalda (Ronda1.h)
//   ON   off   2: lado a lado         (Ronda2.h)
//   off  ON    3: enfrentados         (Ronda3.h)
//   ON   ON    4: salida libre        (Libre.h)
//   DIP3: lado del rival / del giro (off = derecha, ON = izquierda)
// Cada archivo tiene sus velocidades, sus tiempos y su lógica. Las rondas solo
// dirigen el arranque: al terminar (al ver al rival o al agotar su tiempo) el
// combate sigue con la estrategia normal. Ver la línea del borde las corta.
#ifndef PASO_RONDA_H
#define PASO_RONDA_H

#include "movimiento/MandoMotores.h"
#include "movimiento/BusquedaRival.h"
#include "movimiento/ControlAtaque.h"
#include "estrategia/DecisionMovimiento.h"

// Lo que una ronda necesita saber en cada ciclo
struct PasoRonda {
    const DecisionMovimiento& decision;
    MandoMotores& mando;
    IMotor& motor;
    BusquedaRival& busqueda;
    ControlAtaque& ataque;
    unsigned long ahora;
    unsigned long t;     // ms desde que empezó la ronda
    int lado;            // DIP3: 1 = derecha, -1 = izquierda
    int visto;           // sensor de 45° o lateral que lo ve: 1 der, -1 izq, 0 ninguno
    bool deFrente;       // el frontal lo ve (confirmado)
};

// Qué hace la ronda tras este ciclo
enum class ResultadoRonda : unsigned char {
    Sigue,      // sigue la ronda (ya ha mandado los motores)
    Ataca,      // termina con ataque a fondo (ControlAtaque, VelocidadEmpuje)
    Termina     // termina: sigue la estrategia normal
};

#endif
