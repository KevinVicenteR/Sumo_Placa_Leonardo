#ifndef ESTRATEGIA_RED_H
#define ESTRATEGIA_RED_H
#include "estrategia/IEstrategiaCombate.h"
#include "redes/RedLogica.h"
class ControlMovimiento;
class EstrategiaRed: public IEstrategiaCombate {
public:
    DecisionMovimiento decidir(const LecturasSensores&) override;
    void conectar(const ControlMovimiento& c) { contexto = &c; }
    void reiniciar() { ladoUltimo = 0; ultimaVez = inicioFrontal = 0; huboEsquiva = esquivando = false; }
private:
    DecisionMovimiento elegirContraEnemigo(const LecturasSensores&);
    const ControlMovimiento* contexto = nullptr;
    int ladoUltimo = 0;
    unsigned long ultimaVez = 0, inicioFrontal = 0;
    unsigned long inicioEsquiva;
    bool huboEsquiva = false, esquivando = false;
    TipoAccion ultimaEsquiva;
    long sumas[redlogica::NumAcciones];
};
#endif
