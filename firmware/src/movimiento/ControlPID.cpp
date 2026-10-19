#include "movimiento/ControlPID.h"
#include "configuracion/ParametrosRedes.h"

namespace {
constexpr long U = ControlPID::Unidad;
constexpr long KpQ = (long)(KpPID * U * 64 + 0.5f);
constexpr long KiQ = (long)(KiPID * 256 + 0.5f);
constexpr long KdQ = (long)(KdPID * 256 + 0.5f);
constexpr long FiltroQ = (long)(FiltroDerivadaPID * 256 + 0.5f);
constexpr long LimiteIntegral = KiQ > 0 ? U * 256 / KiQ : 0;

long limitar(long x, long limite) {
    return x > limite ? limite : x < -limite ? -limite : x;
}
}

int ControlPID::correccion(int error, unsigned long ahora) {
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
    if (!activo || ahora - tAnterior > PausaReinicioPID) {
        activo = true;
        errorAnterior = error;
        tAnterior = ahora;
        integral = 0;
        derivada = 0;
        return (int)limitar(p, U);
    }
    if (ahora - tAnterior >= PeriodoPID) {
        const long cambio = (long)(error - errorAnterior) * U / 1000;
        derivada += (int)((cambio - derivada) * FiltroQ / 256);
        integral = limitar(integral + (long)error * U / 1000, LimiteIntegral);
        errorAnterior = error;
        tAnterior = ahora;
    }
    return (int)limitar(p + integral * KiQ / 256 + (long)derivada * KdQ / 256, U);
}
