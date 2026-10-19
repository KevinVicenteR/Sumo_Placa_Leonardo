#include <Arduino.h>
#include "hardware/DriverMotores.h"
#include "hardware/PlacaXMotion.h"
#include "sensores/Percepcion.h"
#include "estrategia/EstrategiaReglas.h"
#include "movimiento/ControlMovimiento.h"
#include "robot/ControladorRobot.h"
#include "estrategia/EstrategiaCompetencia.h"
#include "combate/ModoCombate.h"
#include "configuracion/Parametros.h"

PlacaXMotion placa;
DriverMotores motores;
Percepcion percepcion;
EstrategiaReglas estrategiaClasica;
ControlMovimiento controlMovimiento;
EstrategiaCompetencia estrategia(estrategiaClasica, controlMovimiento);
ControladorRobot robot(percepcion, estrategia, controlMovimiento, motores);

ModoCombate modo(placa, motores, percepcion, controlMovimiento, robot);

void setup() {
    modo.iniciar();
}

void loop() {
    modo.actualizar();
}
