// =============================================================================
// MandoMotores.h — Última etapa antes de los motores
// -----------------------------------------------------------------------------
// Todas las órdenes de movimiento pasan por aquí. Esta clase:
//   - limita la velocidad hacia delante (arranque suave, cerca del borde,
//     estimador de borde...),
//   - aplica la rampa (sube la velocidad poco a poco para no patinar),
//   - recuerda la última orden y estima la velocidad real de cada rueda,
//   - calcula el contramando para frenar en seco.
// =============================================================================
#ifndef MANDO_MOTORES_H
#define MANDO_MOTORES_H

#include "hardware/IMotor.h"
#include "Parametros.h"

// Límites de avance que imponen otras partes del movimiento (255 = sin límite)
struct LimitesAvance {
    int salidaSuave = 255;    // tras evadir el borde con un solo sensor
    bool cercaBorde = false;  // un sensor de piso empieza a ver claro
    int estimador = 255;      // el estimador de borde prevé el borde cerca
    bool empujando = false;   // empujando al rival (el estimador puede no limitar)
};

// Contramando para frenar: velocidad de cada rueda y cuánto tiempo aplicarla
struct Freno {
    int izq = 0;
    int der = 0;
    unsigned long duracion = 0;
};

class MandoMotores {
public:
    explicit MandoMotores(int rampaPwmPorMs = RampaPwmPorMs): rampa(rampaPwmPorMs) {}

    // Al principio de cada ciclo: actualiza la rampa, el arranque suave y la
    // velocidad estimada de las ruedas
    void nuevoCiclo(unsigned long ahora, bool conRutina);

    // Manda una orden a los motores. urgente = sin rampa (frenar, escapar...)
    void mover(IMotor& motor, int izq, int der, bool urgente = false);

    // Calcula el contramando para parar las ruedas según su velocidad estimada.
    // Devuelve false si van tan despacio que no hace falta frenar.
    bool calcularFreno(unsigned long duracionMaxima, Freno& freno) const;

    // Las ruedas se mueven hacia delante (según la velocidad estimada)
    bool avanzando() const { return estIzq + estDer > 0; }
    int ordenIzquierda() const { return ordenIzq; }
    int ordenDerecha() const { return ordenDer; }

    LimitesAvance limites;

private:
    int conRampa(int actual, int objetivo) const;
    void estimarVelocidad(unsigned long ahora);

    int rampa;
    // Rampa: cuánto puede subir la orden en este ciclo
    int pasoRampa = 255;
    unsigned long ultimaRampa = 0;
    bool primerCiclo = true;
    // Arranque suave: límite de avance al empezar a moverse
    unsigned long inicioMovimiento = 0;
    int limiteArranque = 255;
    // Última orden enviada a cada motor
    int ordenIzq = 0;
    int ordenDer = 0;
    // Velocidad estimada de cada rueda (orden filtrada, multiplicada por 256)
    long estIzq = 0;
    long estDer = 0;
    unsigned long ultimaEstimacion = 0;
};

#endif
