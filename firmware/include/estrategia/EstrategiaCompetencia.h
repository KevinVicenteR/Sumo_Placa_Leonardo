#ifndef ESTRATEGIA_COMPETENCIA_H
#define ESTRATEGIA_COMPETENCIA_H
#include "estrategia/IEstrategiaCombate.h"
#include "estrategia/EstrategiaRed.h"
#include "movimiento/ControlMovimiento.h"
#include "ParametrosModo4.h"
// Selección persistente por DIP: una rutina cancelada sigue en su modo original.
class EstrategiaCompetencia: public IEstrategiaCombate {
public:
    EstrategiaCompetencia(IEstrategiaCombate& anterior, ControlMovimiento& c)
        : clasica(anterior), control(c) { c.habilitarRedes(); red.conectar(c); }
    DecisionMovimiento decidir(const LecturasSensores& l) override {
        if (!control.mejorasActivas()) return clasica.decidir(l);
        if (!UsarRedLogica || (control.modoCompetencia() == 1 && !UsarRedLogicaModo1) ||
            (control.modoCompetencia() == 3 && !UsarRedLogicaModo3)) {
            DecisionMovimiento d = clasica.decidir(l);
            const int vistos = l.c45Izq + l.frontal + l.c45Der;
            d.errorRival = vistos ? (l.c45Der - l.c45Izq) * PosicionSensor45 / vistos : 0;
            return d;
        }
        if (generacion != control.generacionCombate()) {
            red.reiniciar();
            generacion = control.generacionCombate();
        }
        DecisionMovimiento d = red.decidir(l);
        // Modo 3: de frente contra el rival; si la red quiere esquivar, decisión clásica
        if (control.modoCompetencia() == 3 && !EsquivarModo3 && esEsquiva(d.tipo)) {
            d = clasica.decidir(l);
            const int vistos = l.c45Izq + l.frontal + l.c45Der;
            d.errorRival = vistos ? (l.c45Der - l.c45Izq) * PosicionSensor45 / vistos : 0;
        }
        return d;
    }
private:
    IEstrategiaCombate& clasica;
    const ControlMovimiento& control;
    EstrategiaRed red;
    unsigned char generacion = 0;
};
#endif
