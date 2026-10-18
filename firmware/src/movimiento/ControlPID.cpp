#include "movimiento/ControlPID.h"
#include "ParametrosModo4.h"

namespace {
constexpr long U = ControlPID::Unidad;
// Ganancias en punto fijo. P: KpPID · error, con 64 · Unidad de resolución
constexpr long KpQ = (long)(KpPID * U * 64 + 0.5f);
constexpr long KiQ = (long)(KiPID * 256 + 0.5f);
constexpr long KdQ = (long)(KdPID * 256 + 0.5f);
constexpr long FiltroQ = (long)(FiltroDerivadaPID * 256 + 0.5f);
// Anti-windup: Ki · integral no pasa de ±1
constexpr long LimiteIntegral = KiQ > 0 ? U * 256 / KiQ : 0;

long limitar(long x, long limite) {
    return x > limite ? limite : x < -limite ? -limite : x;
}
}

int ControlPID::correccion(int error, unsigned long ahora) {
    // Con estas ganancias, las fracciones se reducen exactamente. Los sensores
    // producen -1000..1000: todos los intermedios caben en int de 16 bits AVR.
    if (KpQ == 92 && KiQ == 0 && KdQ == 256 && FiltroQ == 128) {
        const int p = error * 23 / 16;
        if (!activo || ahora - tAnterior > PausaReinicioPID) {
            activo = true;
            errorAnterior = error;
            tAnterior = ahora;
            derivada = 0;
            return p < -Unidad ? -Unidad : p > Unidad ? Unidad : p;
        }
        if (ahora - tAnterior >= PeriodoPID) {
            const int delta = error - errorAnterior;
            const int cambio = delta + delta * 3 / 125;
            derivada += (cambio - derivada) / 2;
            errorAnterior = error;
            tAnterior = ahora;
        }
        const int u = p + derivada;
        return u < -Unidad ? -Unidad : u > Unidad ? Unidad : u;
    }

    const long p = (long)error * KpQ / 64;
    // Primer ciclo de ataque o tras una pausa larga: empieza de cero
    if (!activo || ahora - tAnterior > PausaReinicioPID) {
        activo = true;
        errorAnterior = error;
        tAnterior = ahora;
        integral = 0;
        derivada = 0;
        return (int)limitar(p, U);
    }
    // I y D se actualizan una vez por periodo; P responde en cada ciclo
    if (ahora - tAnterior >= PeriodoPID) {
        const long cambio = (long)(error - errorAnterior) * U / 1000;
        derivada += (int)((cambio - derivada) * FiltroQ / 256);
        integral = limitar(integral + (long)error * U / 1000, LimiteIntegral);
        errorAnterior = error;
        tAnterior = ahora;
    }
    return (int)limitar(p + integral * KiQ / 256 + (long)derivada * KdQ / 256, U);
}
