#include "ControlMovimiento.H"
#include "Parametros.H"

void ControlMovimiento::iniciarEvasion(int sentido, unsigned long duracion, unsigned long ahora) {
    // Mientras se siga viendo la línea, la maniobra se reinicia y el retroceso se prolonga
    fase = Fase::Retrocediendo;
    inicioFase = ahora;
    duracionRetroceso = duracion;
    sentidoGiro = sentido;
}

bool ControlMovimiento::continuarEvasion(const DecisionMovimiento& decision, IMotor& motor, unsigned long ahora) {
    if (fase == Fase::Retrocediendo) {
        if (ahora - inicioFase < duracionRetroceso) {
            motor.mover(-VelocidadRetroceso, -VelocidadRetroceso);
            return true;
        }
        fase = Fase::Girando;
        inicioFase = ahora;
    }

    if (fase == Fase::Girando) {
        // Si el enemigo aparece de frente durante el giro, se aborta para atacar
        if (decision.tipo != TipoAccion::AtaqueFrontal && ahora - inicioFase < TiempoGiroEvasion) {
            motor.mover(sentidoGiro * VelocidadMaxima, -sentidoGiro * VelocidadMaxima);
            return true;
        }
        fase = Fase::Libre;
    }

    return false;
}

void ControlMovimiento::ejecutar(const DecisionMovimiento& decision, IMotor& motor, unsigned long ahora) {
    switch (decision.tipo) {
    case TipoAccion::EvadirBordeIzq:
        iniciarEvasion(1, TiempoRetroceso, ahora);
        break;
    case TipoAccion::EvadirBordeDer:
        iniciarEvasion(-1, TiempoRetroceso, ahora);
        break;
    case TipoAccion::EvadirBordeAmbos:
        iniciarEvasion(1, TiempoRetrocesoAmbos, ahora);
        break;
    default:
        break;
    }

    if (continuarEvasion(decision, motor, ahora)) {
        return;
    }

    switch (decision.tipo) {
    case TipoAccion::AtaqueFrontal:
        motor.mover(VelocidadMaxima, VelocidadMaxima);
        break;
    case TipoAccion::CorregirIzq:
        motor.mover(VelocidadCurva, VelocidadMaxima);
        break;
    case TipoAccion::CorregirDer:
        motor.mover(VelocidadMaxima, VelocidadCurva);
        break;
    case TipoAccion::DefensaIzq:
        motor.mover(-VelocidadPivoteLateral, VelocidadMaxima);
        break;
    case TipoAccion::DefensaDer:
        motor.mover(VelocidadMaxima, -VelocidadPivoteLateral);
        break;
    case TipoAccion::Busqueda:
    default:
        motor.mover(VelocidadAvance, VelocidadBusquedaDer);
        break;
    }
}
