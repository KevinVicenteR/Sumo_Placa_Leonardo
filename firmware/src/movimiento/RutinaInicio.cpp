#include "movimiento/RutinaInicio.h"

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
    // Primer ciclo de la ronda
    if (!enCurso) {
        enCurso = true;
        inicio = ahora;
        rivalVisto.reiniciar();
        busqueda.fijarSentido(lado);
    }
    const unsigned long t = ahora - inicio;
    // Lado en el que lo ve un sensor de 45° o lateral (0 = ninguno). AjusteIzq/Der,
    // CorregirIzq/Der y DefensaIzq/Der van seguidos en TipoAccion, alternando lados
    const int k = (int)decision.tipo - (int)TipoAccion::AjusteIzq;
    int visto = k >= 0 && k <= (int)TipoAccion::DefensaDer - (int)TipoAccion::AjusteIzq ? (k & 1 ? 1 : -1) : 0;
    // El frontal lo ve (confirmado). Se mira el sensor y no la decisión: la red
    // puede proponer girar aunque no vea nada
    bool deFrente = rivalVisto.actualizar(decision.enemigoFrente, ahora);
    // Maniobra obligatoria al empezar: sin mirar los sensores de rival
    const unsigned long minimo = rutina == 3 ? Ronda3TiempoMinimo : rutina == 4 ? LibreTiempoMinimo : 0;
    if (t < minimo) {
        visto = 0;
        deFrente = false;
    }
    const PasoRonda p{decision, mando, motor, busqueda, ataque, ahora, t, lado, visto, deFrente};
    const ResultadoRonda r = rutina == 1 ? ronda1.paso(p) : rutina == 2 ? ronda2.paso(p)
                           : rutina == 3 ? ronda3.paso(p) : libre.paso(p);
    if (r == ResultadoRonda::Ataca) {
        ataque.iniciarAtaqueMaximo(ahora);
    }
    if (r != ResultadoRonda::Sigue) {
        rutina = 0;
        return false;
    }
    return true;
}
