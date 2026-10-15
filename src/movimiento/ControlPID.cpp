#include "movimiento/ControlPID.h"
#include "Parametros.h"

namespace {
float limitar(float x, float limite) {
    return x > limite ? limite : x < -limite ? -limite : x;
}
}

float ControlPID::correccion(int error, unsigned long ahora) {
    // Primer ciclo de ataque o tras una pausa larga: empieza de cero
    if (!activo || ahora - tAnterior > PausaReinicioPID) {
        activo = true;
        errorAnterior = error;
        tAnterior = ahora;
        integral = 0;
        derivada = 0;
        salida = KpPID * error;
        return limitar(salida, 1.0f);
    }
    // I y D se actualizan una vez por periodo; P responde en cada ciclo
    if (ahora - tAnterior >= PeriodoPID) {
        const float e = error / 1000.0f;
        const float cambio = (error - errorAnterior) / 1000.0f;
        derivada += FiltroDerivadaPID * (cambio - derivada);
        // Anti-windup: la integral no pasa de ±1
        integral = limitar(integral + e, KiPID > 0 ? 1.0f / KiPID : 0.0f);
        errorAnterior = error;
        tAnterior = ahora;
    }
    salida = KpPID * error + KiPID * integral + KdPID * derivada;
    return limitar(salida, 1.0f);
}
