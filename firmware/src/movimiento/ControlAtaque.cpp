#include "movimiento/ControlAtaque.h"
#include "ParametrosModo4.h"
#include "Parametros.h"

void ControlAtaque::actualizarContacto(const DecisionMovimiento& decision, bool evadiendo,
                                       unsigned long ahora) {
    const bool atacando = esAtaque(decision.tipo) || (esBorde(decision.tipo) && decision.enemigoFrente);
    if (!atacando || evadiendo) {
        viendoFrente = false;
        empujandoRival = false;
    } else if (!empujandoRival) {
        // Cuenta el tiempo con el rival de frente; al llegar a TiempoEmbestida, empuja
        if (!veDeFrente(decision)) {
            viendoFrente = false;
        } else if (!viendoFrente) {
            viendoFrente = true;
            inicioFrente = ahora;
        }
        empujandoRival = viendoFrente && ahora - inicioFrente >= TiempoEmbestida;
    }
}

int ControlAtaque::velocidad(const DecisionMovimiento& decision, unsigned long ahora) {
    // A máxima velocidad mientras lo siga atacando
    if (ataqueMaximo && !esAtaque(decision.tipo)) {
        ataqueMaximo = false;
    }
    // Sin llegar a empujar en TiempoMaxAtaqueLibre: lo que ve puede estar lejos o
    // fuera del dohyo (gente, reflejos); sigue con el ataque normal, que respeta
    // el limitador de borde
    if (ataqueMaximo && !empujandoRival && (unsigned int)((unsigned int)ahora - inicioAtaqueMaximo) >= TiempoMaxAtaqueLibre) {
        ataqueMaximo = false;
    }
    if (ataqueMaximo) {
        return VelocidadEmpuje;
    }
    int ataque = VelocidadAtaque;
    if (empujandoRival || (viendoFrente && TiempoEmbestida == 0)) {
        ataque = VelocidadEmpuje;
    } else if (viendoFrente) {
        // Subida gradual proporcional al tiempo con el rival de frente
        const unsigned long transcurrido = ahora - inicioFrente;
        const unsigned long progreso = transcurrido < TiempoEmbestida ? transcurrido : TiempoEmbestida;
        ataque += (long)(VelocidadEmpuje - VelocidadAtaque) * progreso / TiempoEmbestida;
    }
    // Rival centrado en los tres sensores: a fondo sin esperar a la subida gradual
    if (decision.ataqueDirecto && ataque < VelocidadAtaqueDirecto) {
        ataque = VelocidadAtaqueDirecto;
    }
    return ataque;
}
