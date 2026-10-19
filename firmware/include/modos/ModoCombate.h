// ModoCombate.h — Funcionamiento normal en competencia
// Espera la señal RUN del módulo de arranque con los motores apagados y, al
// recibirla, calibra el piso, elige la rutina de inicio y combate. Con STOP
// vuelve a apagar los motores y guarda el combate en la caja negra (EEPROM).
// STOP o PROG del control remoto: para los motores y reinicia la placa entera
// (como apagarla y encenderla); los LED RX y TX avisan de que está lista para
// el siguiente START.
#ifndef MODO_COMBATE_H
#define MODO_COMBATE_H

#include "hardware/DriverMotores.h"
#include "hardware/PlacaXMotion.h"
#include "hardware/ModuloArranque.h"
#include "hardware/CajaNegra.h"
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
    // Empieza un combate nuevo con el robot ya colocado sobre el negro: lo
    // reinicia todo (como recién encendido) y hace la rutina de los DIP
    void empezarCombate();
    // Reinicia el microcontrolador (como apagar y encender) tras una parada confirmada
    void reiniciarPlaca();
    // LED RX y TX: "listo" mientras espera el START (ver AvisoListoLed)
    void avisoListo(bool encendido);
    void actualizarAvisoListo();

    PlacaXMotion& placa;
    DriverMotores& motores;
    Percepcion& percepcion;
    ControlMovimiento& controlMovimiento;
    ControladorRobot& robot;
    ModuloArranque moduloArranque;
    CajaNegra cajaNegra;
    bool enCombate = false;
    // También cuenta el START durante la espera reglamentaria.
    bool combateEmpezado = false;
    unsigned long inicioCombate = 0;
    // La placa arrancó tras un reinicio por STOP o PROG (parpadeo del aviso)
    bool reiniciadaPorParada = false;
};

#endif
