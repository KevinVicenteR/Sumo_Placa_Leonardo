#include <Arduino.h>
#include "Motor.H"
#include "ControladorRobot.H"
#include "ConfiguracionHardware.H"
#include "Percepcion.H"
#include "EstrategiaCombate.H"
#include "ControlMovimiento.H"
#include "Parametros.H"

Motor motor;
Percepcion percepcion;
EstrategiaCombate estrategia;
ControlMovimiento controlMovimiento;
ControladorRobot robot(percepcion, estrategia, controlMovimiento, motor);
ConfiguracionHardware hardware;

void setup() {
  hardware.inicializarPines();
  motor.detener();

  delay(TiempoInicioReglamentario);
}

void loop() {
  robot.actualizar();
}
