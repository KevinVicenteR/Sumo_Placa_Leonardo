// ManiobraEvasion.h — Alejarse del borde al ver la línea blanca
// Máquina de estados (patrón State, con un enum):
//   Retrocediendo -> Frenando -> Girando -> Asentando -> Libre
//   GirandoSalida -> Girando -> Asentando -> Libre   (si no venía avanzando)
// - Retrocediendo: hasta ver negro un rato seguido (o agotar el tiempo máximo).
// - GirandoSalida: lo mismo pero girando en el sitio, sin moverse hacia atrás.
// - Frenando: motores parados un momento.
// - Girando: se aparta del borde girando en el sitio.
// - Asentando: otra pausa antes de seguir combatiendo.
// Si vuelve a ver la línea a mitad de maniobra, vuelve a retroceder. Si se
// queda parado sobre la línea (TiempoMaxParadoEnLinea), lo intenta de nuevo
// retrocediendo un poco. Si ve al rival fuera de la línea, corta la maniobra
// (CortarEvasionAlVerRival) para ir contra él.
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
    // veniaAvanzando = false: sale girando en el sitio en vez de retroceder.
    bool iniciar(int sentido, unsigned long duracionRetroceso, unsigned long duracionGiro,
                 unsigned long ahora, bool unSensor, bool veniaAvanzando = true);

    // Un ciclo de la maniobra. Devuelve true mientras siga en marcha.
    bool continuar(const DecisionMovimiento& decision, MandoMotores& mando, IMotor& motor,
                   unsigned long ahora, BusquedaRival& busqueda);

    // Al principio de cada ciclo: termina la salida suave cuando toca
    void actualizarSalidaSuave(unsigned long ahora);
    // Límite de avance tras evadir con un solo sensor (255 = sin límite)
    int limiteSalidaSuave() const { return limiteSalida; }

private:
    enum class Fase { Libre, Retrocediendo, GirandoSalida, Frenando, Girando, Asentando };

    // Se separa de la línea (retrocediendo o girando). Devuelve true mientras siga.
    bool separarse(bool linea, MandoMotores& mando, IMotor& motor, unsigned long ahora);
    // En una pausa: si lleva demasiado tiempo parado sobre la línea, vuelve a
    // retroceder un poco. Devuelve true si lo hace.
    bool reintentarSiAtascado(bool linea, MandoMotores& mando, IMotor& motor, unsigned long ahora);

    // Ve al rival (fuera de la línea) el tiempo suficiente para cortar la maniobra
    bool rivalConfirmado(const DecisionMovimiento& decision, bool linea, unsigned long ahora);

    Fase fase = Fase::Libre;
    // Desde cuándo ve al rival durante la maniobra
    bool viendoRival = false;
    unsigned long inicioRival = 0;
    // Último corte de la maniobra por ver al rival
    bool huboCorte = false;
    unsigned long ultimoCorte = 0;
    unsigned long inicioFase = 0;
    unsigned long inicioEvasion = 0;
    // Negro continuo bajo los sensores al retroceder
    bool negroContinuo = false;
    unsigned long inicioNegro = 0;
    // La línea la vio un solo sensor (maniobra más corta y suave)
    bool unSensor = false;
    // Sale girando en el sitio en vez de retroceder
    bool sinRetroceso = false;
    unsigned long duracionRetroceso = 0;
    unsigned long duracionGiro = 0;
    int sentidoGiro = 1;
    // Salida suave tras la maniobra
    bool salidaSuave = false;
    unsigned long inicioSalidaSuave = 0;
    int limiteSalida = 255;
};

#endif
