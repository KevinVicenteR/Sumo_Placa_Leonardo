// EstrategiaCombate.h — Base común de todas las estrategias
// Patrón Método Plantilla (Template Method): decidir() fija los pasos que toda
// estrategia respeta y deja a cada subclase solo la parte que cambia:
//   1. Si hay línea blanca, evadir el borde (siempre tiene prioridad).
//   2. Rival centrado: atacar de frente; la red puede sustituirlo por una esquiva.
//   3. Si no, la subclase decide qué hacer con el rival (elegirContraEnemigo).
#ifndef ESTRATEGIA_COMBATE_H
#define ESTRATEGIA_COMBATE_H

#include "estrategia/IEstrategiaCombate.h"
#include "estrategia/IContextoCombate.h"

class EstrategiaCombate: public IEstrategiaCombate {
public:
    DecisionMovimiento decidir(const LecturasSensores& lecturas) override;
    // Información del movimiento para las estrategias que la usan (EstrategiaRed)
    void conectar(const IContextoCombate& c) { contexto = &c; }

protected:
    const IContextoCombate* contexto = nullptr;
    // Las estrategias aprendidas pueden decidir esquivar un rival centrado.
    virtual bool priorizarAtaqueFrontal() const { return true; }
    // Paso 3: lo que cambia entre estrategias
    virtual DecisionMovimiento elegirContraEnemigo(const LecturasSensores& lecturas) = 0;

private:
    DecisionMovimiento elegir(const LecturasSensores& lecturas);
    // Posición del rival para el PID (ver DecisionMovimiento::errorRival)
    static int errorRival(const LecturasSensores& lecturas);
};

#endif
