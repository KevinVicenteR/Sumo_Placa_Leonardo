#include "movimiento/ControlPID.h"
#include "Parametros.h"

float ControlPID::correccion(int error, unsigned long ahora) {
    // Primer ciclo de ataque o tras una pausa larga: sin derivada (evita un salto)
    if (!activo || ahora - tAnterior > PausaReinicioPID) {
        activo = true;
        errorAnterior = error;
        tAnterior = ahora;
        derivada = 0;
    } else if (ahora > tAnterior) {
        // millis() solo avanza de ms en ms: la derivada se recalcula cada ms
        derivada = (float)(error - errorAnterior) / (float)(ahora - tAnterior) / 1000.0f;
        errorAnterior = error;
        tAnterior = ahora;
    }
    const float u = KpPID * error + KdPID * derivada;
    return u > 1.0f ? 1.0f : u < -1.0f ? -1.0f : u;
}
