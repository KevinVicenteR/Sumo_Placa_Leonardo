#ifndef CONTROL_ATAQUE_H
#define CONTROL_ATAQUE_H

#include "estrategia/DecisionMovimiento.h"

class ControlAtaque {
public:
    void actualizarContacto(const DecisionMovimiento& decision, bool evadiendo, unsigned long ahora);
    int velocidad(const DecisionMovimiento& decision, unsigned long ahora);

    void iniciarAtaqueMaximo(unsigned long ahora) {
        ataqueMaximo = true;
        inicioAtaqueMaximo = (unsigned int)ahora;
    }
    void olvidarFrente() { viendoFrente = false; empujandoRival = false; }

    bool empujando() const { return empujandoRival; }

private:
    bool viendoFrente = false;
    unsigned long inicioFrente = 0;
    bool empujandoRival = false;
    bool ataqueMaximo = false;
    unsigned int inicioAtaqueMaximo = 0;
};

#endif
