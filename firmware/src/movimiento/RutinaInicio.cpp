#include "movimiento/RutinaInicio.h"
#include "ParametrosModo4.h"

void RutinaInicio::iniciar(int rutinaElegida, int ladoElegido) {
    rutina = rutinaElegida;
    lado = ladoElegido < 0 ? -1 : 1;
    enCurso = false;
    conRutina = rutinaElegida != 0;
}

bool RutinaInicio::continuar(const DecisionMovimiento& decision, MandoMotores& mando, IMotor& motor,
                             unsigned long ahora, BusquedaRival& busqueda, ControlAtaque& ataque) {
    if (rutina < 1 || rutina > 4) {
        return false;
    }
    // Primer ciclo del modo
    if (!enCurso) {
        enCurso = true;
        inicio = ahora;
        rivalVisto.reiniciar();
        busqueda.fijarSentido(lado);
    }
    if (!modoRumania(decision, mando, motor, ahora, ahora - inicio, ataque)) {
        rutina = 0;
        return false;
    }
    return true;
}

// Modos de Rumania (movimiento/rumania/Modo1Defensivo.h ... Modo4Agresivo.h). En
// cualquier modo, el sensor frontal lo ve (confirmado): termina y ataca a fondo
// (salvo lo que decida el modo). Se mira el sensor y no la decisión: la red puede
// proponer girar aunque no vea nada.
bool RutinaInicio::modoRumania(const DecisionMovimiento& decision, MandoMotores& mando, IMotor& motor,
                               unsigned long ahora, unsigned long t, ControlAtaque& ataque) {
    // Lado en el que lo ve un sensor de 45° o lateral (0 = ninguno). AjusteIzq/Der,
    // CorregirIzq/Der y DefensaIzq/Der van seguidos en TipoAccion, alternando lados
    const int k = (int)decision.tipo - (int)TipoAccion::AjusteIzq;
    int visto = k >= 0 && k <= (int)TipoAccion::DefensaDer - (int)TipoAccion::AjusteIzq ? (k & 1 ? 1 : -1) : 0;
    bool deFrente = rivalVisto.actualizar(decision.enemigoFrente, ahora);
    // Primero, siempre la maniobra del modo: sin mirar los sensores de rival
    const unsigned long minimo = rutina == 1 ? Modo1TiempoMinimo : rutina == 2 ? Modo2TiempoMinimo
                               : rutina == 3 ? Modo3TiempoMinimo : Modo4TiempoMinimo;
    if (t < minimo) {
        visto = 0;
        deFrente = false;
    }
    const PasoModo p{decision, mando, motor, ahora, t, lado, visto, deFrente};
    const ResultadoModo r = rutina == 1 ? modo1.paso(p) : rutina == 2 ? modo2.paso(p)
                          : rutina == 3 ? modo3.paso(p) : modo4.paso(p);
    if (r == ResultadoModo::Ataca) {
        ataque.iniciarAtaqueMaximo(ahora);
    }
    return r == ResultadoModo::Sigue;
}
