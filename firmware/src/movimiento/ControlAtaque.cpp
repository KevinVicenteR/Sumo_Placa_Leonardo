#include "movimiento/ControlAtaque.h"
#include "Parametros.h"
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

void ControlAtaque::iniciarEmbestida(unsigned long ahora) {
    embestidaInicio = true;
    ataqueDeRutina = true;
    finEmbestidaInicio = ahora + TiempoEmbestidaInicio;
}

int ControlAtaque::velocidad(const DecisionMovimiento& decision, unsigned long ahora) {
    // La embestida de la rutina dura mientras siga atacando y no se acabe su tiempo
    if (embestidaInicio && (!esAtaque(decision.tipo) || (long)(ahora - finEmbestidaInicio) >= 0)) {
        embestidaInicio = false;
    }
    if (ataqueDeRutina && !esAtaque(decision.tipo)) {
        ataqueDeRutina = false;
    }
    // Modo libre: a máxima velocidad mientras lo siga atacando
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
    if (embestidaInicio) {
        // Subida proporcional al tiempo que lleva embistiendo
        const unsigned long transcurrido = ahora - (finEmbestidaInicio - TiempoEmbestidaInicio);
        ataque = VelocidadEmbestidaInicio +
                 (long)(VelocidadEmbestidaMaxima - VelocidadEmbestidaInicio) * transcurrido / TiempoEmbestidaInicio;
    } else if (empujandoRival || (viendoFrente && TiempoEmbestida == 0)) {
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
    // Rondas 1 y 2: tras la embestida conserva su velocidad propia
    if (ataqueDeRutina && !embestidaInicio && ataque > VelocidadAtaqueRound12) {
        ataque = VelocidadAtaqueRound12;
    }
    return ataque;
}
