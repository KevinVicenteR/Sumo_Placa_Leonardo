#include "maniobras/ControlAtaque.h"
#include "configuracion/ParametrosRedes.h"
#include "configuracion/Parametros.h"

void ControlAtaque::actualizarContacto(const DecisionMovimiento& decision, bool evadiendo,
                                       unsigned long ahora) {
    const bool atacando = esAtaque(decision.tipo) || (esBorde(decision.tipo) && decision.enemigoFrente);
    if (!atacando || evadiendo) {
        viendoFrente = false;
        empujandoRival = false;
    } else if (!empujandoRival) {
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
    if (ataqueMaximo && !esAtaque(decision.tipo)) {
        ataqueMaximo = false;
    }
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
        const unsigned long transcurrido = ahora - inicioFrente;
        const unsigned long progreso = transcurrido < TiempoEmbestida ? transcurrido : TiempoEmbestida;
        ataque += (long)(VelocidadEmpuje - VelocidadAtaque) * progreso / TiempoEmbestida;
    }
    if (decision.ataqueDirecto && ataque < VelocidadAtaqueDirecto) {
        ataque = VelocidadAtaqueDirecto;
    }
    return ataque;
}
