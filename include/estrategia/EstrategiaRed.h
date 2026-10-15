// EstrategiaRed.h — Estrategia de la red de lógica (RedLogica.h)
// Junta las entradas de la red (sensores, memoria del rival y contexto del
// movimiento) y hace la acción ganadora. Se activa con UsarRedLogica
// (Parametros.h). El borde conserva prioridad; la red puede esquivar también
// cuando los tres sensores delanteros ven al rival.
#ifndef ESTRATEGIA_RED_H
#define ESTRATEGIA_RED_H

#include "estrategia/EstrategiaCombate.h"
#include "estrategia/RedLogica.h"

class EstrategiaRed: public EstrategiaCombate {
public:
    // Suma de cada salida en el último ciclo (telemetría y tests)
    long suma(int salida) const { return sumas[salida]; }

protected:
    bool priorizarAtaqueFrontal() const override { return false; }
    DecisionMovimiento elegirContraEnemigo(const LecturasSensores& lecturas) override;

private:
    int ladoUltimo = 0;               // último lado donde se vio: 0 izq, 1 der
    unsigned long ultimaVez = 0;      // última vez que vio al rival
    unsigned long inicioFrontal = 0;  // desde cuándo lo ve el frontal sin parar
    unsigned long inicioEsquiva = 0;
    bool huboEsquiva = false;
    TipoAccion ultimaEsquiva = TipoAccion::EsquivarIzq;
    bool esquivando = false;          // la última decisión fue esquivar
    long sumas[redlogica::NumAcciones] = {};
};

#endif
