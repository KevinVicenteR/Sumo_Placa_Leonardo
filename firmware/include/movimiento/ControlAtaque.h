// ControlAtaque.h — Velocidad de ataque
// - Con el rival de frente, la velocidad sube de VelocidadAtaque a
//   VelocidadEmpuje durante TiempoEmbestida; al completarse, está "empujando".
// - Con el rival en los tres sensores delanteros, VelocidadAtaqueDirecto.
// - Al terminar un modo de Rumania con el rival de frente: a máxima velocidad
//   (VelocidadEmpuje) mientras lo ataque.
#ifndef CONTROL_ATAQUE_H
#define CONTROL_ATAQUE_H

#include "estrategia/DecisionMovimiento.h"

class ControlAtaque {
public:
    // Al principio de cada ciclo: ¿sigue el rival de frente? ¿ya está empujando?
    void actualizarContacto(const DecisionMovimiento& decision, bool evadiendo, unsigned long ahora);
    // Velocidad para la acción de ataque de este ciclo
    int velocidad(const DecisionMovimiento& decision, unsigned long ahora);

    // Ataque a máxima velocidad (VelocidadEmpuje) mientras lo ataque
    void iniciarAtaqueMaximo(unsigned long ahora) {
        ataqueMaximo = true;
        inicioAtaqueMaximo = (unsigned int)ahora;
    }
    // Pierde el contacto (por ejemplo, al evadir el borde)
    void olvidarFrente() { viendoFrente = false; empujandoRival = false; }

    bool empujando() const { return empujandoRival; }

private:
    bool viendoFrente = false;
    unsigned long inicioFrente = 0;
    bool empujandoRival = false;
    // Ataque a máxima velocidad
    bool ataqueMaximo = false;
    unsigned int inicioAtaqueMaximo = 0;   // 16 bits: basta para TiempoMaxAtaqueLibre
};

#endif
