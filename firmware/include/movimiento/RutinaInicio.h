// RutinaInicio.h — Primer movimiento de cada round (elegido con los DIP)
// Cada ronda está en su archivo de movimiento/rondas/ con sus velocidades,
// sus tiempos y su lógica:
//   1 = espalda con espalda (Ronda1.h)   2 = lado a lado (Ronda2.h)
//   3 = enfrentados (Ronda3.h)           4 = salida libre (Libre.h)
//   0 = sin ronda.
// Ver la línea cancela la ronda (lo hace ControlMovimiento).
#ifndef RUTINA_INICIO_H
#define RUTINA_INICIO_H

#include "movimiento/MandoMotores.h"
#include "movimiento/Confirmacion.h"
#include "movimiento/BusquedaRival.h"
#include "movimiento/ControlAtaque.h"
#include "estrategia/DecisionMovimiento.h"
#include "movimiento/rondas/Ronda1.h"
#include "movimiento/rondas/Ronda2.h"
#include "movimiento/rondas/Ronda3.h"
#include "movimiento/rondas/Libre.h"

class RutinaInicio {
public:
    // lado: 1 = derecha, -1 = izquierda (DIP3)
    void iniciar(int rutinaElegida, int lado);
    void cancelar() { rutina = 0; }

    // Ronda en marcha (0 = ninguna)
    int actual() const { return rutina; }
    // Se eligió una ronda al empezar (desactiva el arranque suave)
    bool elegida() const { return conRutina; }

    // Un ciclo de la ronda. Devuelve true mientras siga en marcha.
    bool continuar(const DecisionMovimiento& decision, MandoMotores& mando, IMotor& motor,
                   unsigned long ahora, BusquedaRival& busqueda, ControlAtaque& ataque);

private:
    Ronda1 ronda1;
    Ronda2 ronda2;
    Ronda3 ronda3;
    Libre libre;

    int rutina = 0;
    int lado = 1;
    bool conRutina = false;
    bool enCurso = false;
    unsigned long inicio = 0;
    // El frontal ve al rival (confirmado), para las rondas 3 y libre
    Confirmacion rivalVisto;
};

#endif
