// RutinaInicio.h — Primer movimiento de cada round: el modo de Rumania elegido
// con los DIP (movimiento/rumania/: Modo1Defensivo ... Modo4Agresivo).
//   DIP1 DIP2  modo
//   off  off   1: de frente, defensivo
//   ON   off   2: de frente, contragolpe
//   off  ON    3: de lado (DIP3: rival a la derecha off / izquierda ON)
//   ON   ON    4: de espaldas, agresivo
// Ver la línea cancela el modo (lo hace ControlMovimiento).
#ifndef RUTINA_INICIO_H
#define RUTINA_INICIO_H

#include "movimiento/MandoMotores.h"
#include "movimiento/Confirmacion.h"
#include "movimiento/BusquedaRival.h"
#include "movimiento/ControlAtaque.h"
#include "estrategia/DecisionMovimiento.h"
#include "movimiento/rumania/Modo1Defensivo.h"
#include "movimiento/rumania/Modo2Contragolpe.h"
#include "movimiento/rumania/Modo3Lado.h"
#include "movimiento/rumania/Modo4Agresivo.h"

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
    // Modos de Rumania (movimiento/rumania/): cada uno en su archivo
    bool modoRumania(const DecisionMovimiento& decision, MandoMotores& mando, IMotor& motor,
                     unsigned long ahora, unsigned long t, ControlAtaque& ataque);
    Modo1Defensivo modo1;
    Modo2Contragolpe modo2;
    Modo3Lado modo3;
    Modo4Agresivo modo4;

    int rutina = 0;
    int lado = 1;
    bool conRutina = false;
    bool enCurso = false;
    unsigned long inicio = 0;
    Confirmacion rivalVisto;
};

#endif
