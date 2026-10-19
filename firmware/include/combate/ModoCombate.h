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

    void iniciar();
    void actualizar();

private:
    void empezarCombate();
    void reiniciarPlaca();
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
    bool combateEmpezado = false;
    unsigned long inicioCombate = 0;
    bool reiniciadaPorParada = false;
};

#endif
