// =============================================================================
// ManiobraEvasion.h — Alejarse del borde al ver la línea blanca
// -----------------------------------------------------------------------------
// Máquina de estados (patrón State, con un enum):
//   Retrocediendo -> Frenando -> Girando -> Asentando -> Libre
// - Retrocediendo: hasta ver negro un rato seguido (o agotar el tiempo máximo).
// - Frenando: motores parados un momento.
// - Girando: se aparta del borde girando en el sitio.
// - Asentando: otra pausa antes de seguir combatiendo.
// Si vuelve a ver la línea a mitad de maniobra, vuelve a retroceder.
// =============================================================================
#ifndef MANIOBRA_EVASION_H
#define MANIOBRA_EVASION_H

#include "movimiento/MandoMotores.h"
#include "movimiento/BusquedaRival.h"
#include "estrategia/DecisionMovimiento.h"

class ManiobraEvasion {
public:
    // La maniobra está en marcha
    bool activa() const { return fase != Fase::Libre; }

    // Empieza una evasión. Devuelve false si ya había una en marcha (en ese
    // caso solo la actualiza si ahora la línea la ven los dos sensores).
    bool iniciar(int sentido, unsigned long duracionRetroceso, unsigned long duracionGiro,
                 unsigned long ahora, bool unSensor);

    // Un ciclo de la maniobra. Devuelve true mientras siga en marcha.
    bool continuar(const DecisionMovimiento& decision, MandoMotores& mando, IMotor& motor,
                   unsigned long ahora, BusquedaRival& busqueda);

    // Al principio de cada ciclo: termina la salida suave cuando toca
    void actualizarSalidaSuave(unsigned long ahora);
    // Límite de avance tras evadir con un solo sensor (255 = sin límite)
    int limiteSalidaSuave() const { return limiteSalida; }

private:
    enum class Fase { Libre, Retrocediendo, Frenando, Girando, Asentando };

    Fase fase = Fase::Libre;
    unsigned long inicioFase = 0;
    unsigned long inicioEvasion = 0;
    // Negro continuo bajo los sensores al retroceder
    bool negroContinuo = false;
    unsigned long inicioNegro = 0;
    // La línea la vio un solo sensor (maniobra más corta y suave)
    bool unSensor = false;
    unsigned long duracionRetroceso = 0;
    unsigned long duracionGiro = 0;
    int sentidoGiro = 1;
    // Salida suave tras la maniobra
    bool salidaSuave = false;
    unsigned long inicioSalidaSuave = 0;
    int limiteSalida = 255;
};

#endif
