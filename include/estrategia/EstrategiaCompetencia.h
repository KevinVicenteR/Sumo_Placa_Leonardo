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
        bool usarRed = UsarRedLogica && !(control.modoCompetencia() == 1 && !UsarRedLogicaModo1) &&
                       !(control.modoCompetencia() == 3 && !UsarRedLogicaModo3);
        DecisionMovimiento d;
        if (usarRed) {
            if (generacion != control.generacionCombate()) {
                red.reiniciar();
                generacion = control.generacionCombate();
            }
            d = red.decidir(l);
            // Modo 3: de frente contra el rival; si la red quiere esquivar, decisión clásica
            usarRed = !(control.modoCompetencia() == 3 && !EsquivarModo3 && esEsquiva(d.tipo));
        }
        if (!usarRed) {
            d = clasica.decidir(l);
            d.errorRival = errorRivalPID(l);
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
