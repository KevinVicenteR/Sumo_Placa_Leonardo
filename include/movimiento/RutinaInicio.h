// RutinaInicio.h — Primer movimiento de cada round (elegido con los DIP)
//   1 = espalda con espalda: media vuelta en el sitio hacia el lado de DIP3
//       hasta ver al rival de frente (como en la rama con_control).
//   2 = lado a lado: (opcional) avanza para esquivar y gira hacia el rival.
//   3 = enfrentados: avanza hacia el centro y espera al rival.
//   4 = salida libre (en cualquier parte del dohyo): gira en el sitio hasta que
//       cualquier sensor vea al rival.
//   5-8 = estrategias de espera con colocación libre (Rumania): contragolpe,
//       emboscada a 45°, de lado y espera con esquiva (ver Parametros.h).
//   0 = sin rutina.
// Al encontrar al rival (rondas 1 y 2) frena el giro y empieza la embestida.
// Ver la línea cancela la rutina (lo hace ControlMovimiento).
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
    void decidirLado(const DecisionMovimiento& decision, BusquedaRival& busqueda);
    bool avanzarAlCentro(const DecisionMovimiento& decision, MandoMotores& mando, IMotor& motor,
                         unsigned long ahora, unsigned long t, ControlAtaque& ataque);
    bool explorar(const DecisionMovimiento& decision, MandoMotores& mando, IMotor& motor,
                  unsigned long ahora, unsigned long t);
    // Estrategias de espera (rutinas 5 a 8)
    bool esperarRival(const DecisionMovimiento& decision, MandoMotores& mando, IMotor& motor,
                      unsigned long ahora, unsigned long t, ControlAtaque& ataque);
    bool pivotarHaciaRival(const DecisionMovimiento& decision, MandoMotores& mando, IMotor& motor,
                           unsigned long ahora, ControlAtaque& ataque);
    // Cambia a otra rutina (al agotar la espera)
    bool pasarA(int nueva, unsigned long ahora, MandoMotores& mando, IMotor& motor);
    void empezarFase(int nueva, unsigned long ahora);

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
    // Estrategias de espera: 0 esperando, 1 pivotando hacia el rival, 2 esquivando
    int fase = 0;
    unsigned long inicioFase = 0;
    int ladoGolpe = 1;
    bool viendoLateral = false;
    unsigned long inicioLateral = 0;
};

#endif
