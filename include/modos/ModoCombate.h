// ModoCombate.h — Funcionamiento normal en competencia
// -----------------------------------------------------------------------------
// Espera la señal RUN del módulo de arranque con los motores apagados y, al
// recibirla, calibra el piso, elige la rutina de inicio y combate. Con STOP
// vuelve a apagar los motores.
// Con la bandera MONITOREO_COMBATE también envía telemetría por USB.
#ifndef MODO_COMBATE_H
#define MODO_COMBATE_H

#include "hardware/DriverMotores.h"
#include "hardware/PlacaXMotion.h"
#include "hardware/ModuloArranque.h"
#include "sensores/Percepcion.h"
#include "movimiento/ControlMovimiento.h"
#include "robot/ControladorRobot.h"

class ModoCombate {
public:
    ModoCombate(PlacaXMotion& placa, DriverMotores& motores, Percepcion& percepcion,
                ControlMovimiento& controlMovimiento, ControladorRobot& robot);

    void iniciar();     // en setup()
    void actualizar();  // en loop()

private:
    // Empieza (o reempieza) un combate con el robot ya colocado sobre el negro
    void empezarCombate();
    void enviarTelemetria();

    PlacaXMotion& placa;
    DriverMotores& motores;
    Percepcion& percepcion;
    ControlMovimiento& controlMovimiento;
    ControladorRobot& robot;
    ModuloArranque moduloArranque;
    bool enCombate = false;
#if defined(MONITOREO_COMBATE)
    unsigned long cicloMaximo = 0;   // ciclo más lento (µs) desde la última traza
    unsigned long ultimaTraza = 0;
#endif
};

#endif
