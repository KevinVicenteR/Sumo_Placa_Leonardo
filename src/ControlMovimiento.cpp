#include <Arduino.h>
#include "ControlMovimiento.H"
#include "Pines.H"
#include "UtilMatematica.H"

namespace {
constexpr unsigned long RETROCESO_MIN_MS = 220;
constexpr unsigned long PISO_SEGURO_MS = 80;
constexpr unsigned long GIRO_EVASION_MS = 180;
constexpr unsigned long PAUSA_SEGURA_MS = 100;
// Un solo sensor: girar hasta que el piso lleve este tiempo limpio.
constexpr unsigned long PIVOTE_BORDE_MS = 90;
constexpr int16_t CORRECCION_LIMITE = 40;
constexpr uint8_t BUSQUEDA_SESGO_MASCARA = 0x80;

void moverSuave(IMotor& motor, int16_t baseIzq, int16_t baseDer, int16_t correccion) {
    motor.mover(limitar<int16_t>(baseIzq - correccion, -VelocidadMaxima, VelocidadMaxima),
                limitar<int16_t>(baseDer + correccion, -VelocidadMaxima, VelocidadMaxima));
}
}

int16_t ControlMovimiento::calcularPID(ReguladorPID& pid, int8_t error) {
    const int16_t derivada = error - pid.errorAnterior;
    pid.errorAnterior = error;
    return limitar<int16_t>(error * 8 + derivada * 3, -CORRECCION_LIMITE, CORRECCION_LIMITE);
}

int8_t ControlMovimiento::obtenerSesgoBusqueda() const {
    return (estadoBusqueda & BUSQUEDA_SESGO_MASCARA) ? -1 : 1;
}

void ControlMovimiento::actualizarBusqueda(int8_t error) const {
    if (error < 0) estadoBusqueda &= static_cast<uint8_t>(~BUSQUEDA_SESGO_MASCARA);
    else if (error > 0) estadoBusqueda |= BUSQUEDA_SESGO_MASCARA;
}

void ControlMovimiento::ejecutarBusqueda(IMotor& motor) const {
    const int giro = obtenerSesgoBusqueda() * VelocidadBusquedaDer;
    motor.mover(-giro, giro);
}

void ControlMovimiento::ejecutar(const DecisionMovimiento& decision, IMotor& motor) const {
    const unsigned long ahora = millis();
    const bool bordeAmbos = decision.tipo == TipoAccion::EvadirBordeAmbos;
    const bool bordeUno = decision.tipo == TipoAccion::EvadirBordeIzq ||
                          decision.tipo == TipoAccion::EvadirBordeDer;
    const bool borde = bordeAmbos || bordeUno;

    // Un sensor: solo cambiar de orientación. Ambos sensores, o el lado contrario
    // apareciendo durante un pivote (esquina), obligan a retirarse.
    // El enemigo nunca puede cancelar estas secuencias.
    const bool esquina = bordeUno && faseEvasion == 4 && decision.tipo != tipoEvasionActual;
    if (bordeAmbos || esquina || (bordeUno && faseEvasion == 1)) {
        if (faseEvasion != 1) {
            faseEvasion = 1;
            tiempoInicioEvasion = ahora;
            regulador = ReguladorPID{};
        }
        tipoEvasionActual = decision.tipo;
        tiempoUltimoBorde = ahora;
    } else if (bordeUno) {
        if (faseEvasion != 4) regulador = ReguladorPID{};
        faseEvasion = 4;
        tipoEvasionActual = decision.tipo;
        tiempoUltimoBorde = ahora;
    }

    if (faseEvasion == 4) {
        // Pivote sobre la rueda del lado del borde: esa rueda casi no avanza y la
        // contraria retrocede fuerte, frenando la inercia mientras gira la nariz.
        if (tipoEvasionActual == TipoAccion::EvadirBordeIzq) {
            motor.mover(-VelocidadMinima, -VelocidadRetroceso);
        } else {
            motor.mover(-VelocidadRetroceso, -VelocidadMinima);
        }
        if (!borde && ahora - tiempoUltimoBorde >= PIVOTE_BORDE_MS) faseEvasion = 0;
        return;
    }
    if (faseEvasion == 1) {
        // Ambas ruedas atrás: no pivotar con una rueda aún sobre la línea.
        motor.mover(-VelocidadRetroceso, -VelocidadRetroceso);
        if (!borde && ahora - tiempoInicioEvasion >= RETROCESO_MIN_MS &&
            ahora - tiempoUltimoBorde >= PISO_SEGURO_MS) {
            faseEvasion = 2;
            tiempoInicioEvasion = ahora;
        }
        return;
    }
    if (faseEvasion == 2) {
        const int signo = tipoEvasionActual == TipoAccion::EvadirBordeIzq ? 1 :
                          tipoEvasionActual == TipoAccion::EvadirBordeDer ? -1 : obtenerSesgoBusqueda();
        motor.mover(signo * VelocidadGiro, -signo * VelocidadGiro);
        if (ahora - tiempoInicioEvasion >= GIRO_EVASION_MS) {
            faseEvasion = 3;
            tiempoInicioEvasion = ahora;
        }
        return;
    }
    if (faseEvasion == 3) {
        motor.mover(0, 0);
        if (ahora - tiempoInicioEvasion >= PAUSA_SEGURA_MS) faseEvasion = 0;
        return;
    }

    if (decision.error != 0) actualizarBusqueda(decision.error);
    const int16_t correccion = calcularPID(regulador, decision.error);
    switch (decision.tipo) {
    case TipoAccion::AtaqueFrontal:
        moverSuave(motor, VelocidadAtaqueFrontal, VelocidadAtaqueFrontal, correccion);
        break;
    case TipoAccion::CorregirIzq:
        moverSuave(motor, VelocidadCurva, VelocidadAtaqueFrontal, correccion);
        break;
    case TipoAccion::CorregirDer:
        moverSuave(motor, VelocidadAtaqueFrontal, VelocidadCurva, correccion);
        break;
    case TipoAccion::DefensaIzq:
    case TipoAccion::AtaqueLateralIzq:
        motor.mover(-VelocidadPivoteLateral, VelocidadPivoteLateral);
        break;
    case TipoAccion::DefensaDer:
    case TipoAccion::AtaqueLateralDer:
        motor.mover(VelocidadPivoteLateral, -VelocidadPivoteLateral);
        break;
    case TipoAccion::Busqueda:
        regulador = ReguladorPID{};
        ejecutarBusqueda(motor);
        break;
    default:
        motor.mover(0, 0);
        break;
    }
    ultimaAccion = decision.tipo;
}
