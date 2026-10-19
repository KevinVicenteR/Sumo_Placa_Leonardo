// main.cpp — Punto de entrada del firmware del robot de minisumo (Rumania)
// Aquí se crean todas las piezas del robot y se conectan entre sí (raíz de
// composición).
//
// Estructura de carpetas:
//   hardware/    pines, motores, interruptores y módulo de arranque
//   sensores/    lectura y filtrado de los sensores de piso y de enemigo
//   estrategia/  qué hacer según lo que se ve
//   movimiento/  cómo moverse para hacerlo (modos de Rumania, maniobras y
//                límites de velocidad)
//   robot/       ciclo percibir -> decidir -> actuar
//   modos/       combate (módulo de arranque, STOP/PROG)
#include <Arduino.h>
#include "hardware/DriverMotores.h"
#include "hardware/PlacaXMotion.h"
#include "sensores/Percepcion.h"
#include "estrategia/EstrategiaReglas.h"
#include "movimiento/ControlMovimiento.h"
#include "robot/ControladorRobot.h"
#include "estrategia/EstrategiaCompetencia.h"
#include "modos/ModoCombate.h"
#include "Parametros.h"

// --- Piezas del robot ---
PlacaXMotion placa;
DriverMotores motores;
Percepcion percepcion;
EstrategiaReglas estrategiaClasica;
ControlMovimiento controlMovimiento;
EstrategiaCompetencia estrategia(estrategiaClasica, controlMovimiento);
ControladorRobot robot(percepcion, estrategia, controlMovimiento, motores);

// --- Modo de combate ---
ModoCombate modo(placa, motores, percepcion, controlMovimiento, robot);

void setup() {
    modo.iniciar();
}

void loop() {
    modo.actualizar();
}
