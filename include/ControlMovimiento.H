#ifndef CONTROL_MOVIMIENTO_H
#define CONTROL_MOVIMIENTO_H

#include "IControlMovimiento.H"

// Ejecuta decisiones sin bloquear. La evasión de borde es una maniobra
// temporizada (retroceso -> giro) que avanza un paso en cada llamada.
class ControlMovimiento: public IControlMovimiento {
public:
    void ejecutar(const DecisionMovimiento& decision, IMotor& motor, unsigned long ahora) override;

private:
    enum class Fase { Libre, Retrocediendo, Girando };

    // sentido: +1 gira a la derecha (horario), -1 a la izquierda
    void iniciarEvasion(int sentido, unsigned long duracionRetroceso, unsigned long ahora);
    bool continuarEvasion(const DecisionMovimiento& decision, IMotor& motor, unsigned long ahora);

    Fase fase = Fase::Libre;
    unsigned long inicioFase = 0;
    unsigned long duracionRetroceso = 0;
    int sentidoGiro = 1;
};

#endif
