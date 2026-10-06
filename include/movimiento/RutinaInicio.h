// =============================================================================
// RutinaInicio.h — Primer movimiento de cada round (elegido con los DIP)
// -----------------------------------------------------------------------------
//   1 = espalda con espalda: media vuelta hasta ver al rival de frente.
//   2 = lado a lado: pivota hacia el lado del rival.
//   3 = enfrentados: avanza hacia el centro y espera al rival.
//   0 = sin rutina.
// Al encontrar al rival (rondas 1 y 2) frena el giro y empieza la embestida.
// Ver la línea cancela la rutina (lo hace ControlMovimiento).
// =============================================================================
#ifndef RUTINA_INICIO_H
#define RUTINA_INICIO_H

#include "movimiento/MandoMotores.h"
#include "movimiento/Confirmacion.h"
#include "movimiento/BusquedaRival.h"
#include "movimiento/ControlAtaque.h"
#include "estrategia/DecisionMovimiento.h"

class RutinaInicio {
public:
    // lado: 1 = derecha, -1 = izquierda (DIP3)
    void iniciar(int rutinaElegida, int lado);
    void cancelar() { rutina = 0; }

    // Rutina en marcha (0 = ninguna)
    int actual() const { return rutina; }
    // Se eligió una rutina al empezar (desactiva el arranque suave)
    bool elegida() const { return conRutina; }

    // Un ciclo de la rutina. Devuelve true mientras siga en marcha.
    bool continuar(const DecisionMovimiento& decision, MandoMotores& mando, IMotor& motor,
                   unsigned long ahora, BusquedaRival& busqueda, ControlAtaque& ataque);

private:
    bool girar(const DecisionMovimiento& decision, MandoMotores& mando, IMotor& motor,
               unsigned long ahora, unsigned long t, BusquedaRival& busqueda, ControlAtaque& ataque);
    bool avanzarAlCentro(const DecisionMovimiento& decision, MandoMotores& mando, IMotor& motor,
                         unsigned long ahora, unsigned long t);

    int rutina = 0;
    int lado = 1;
    bool conRutina = false;
    bool enCurso = false;
    unsigned long inicio = 0;
    // Ronda 2: el lado ya lo decidió un sensor lateral
    bool ladoDecidido = false;
    Confirmacion rivalVisto;
    // Frenado del giro al encontrar al rival
    bool frenandoGiro = false;
    unsigned long inicioFreno = 0;
    Freno freno;
};

#endif
