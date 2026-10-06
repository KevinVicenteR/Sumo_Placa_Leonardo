// main.cpp — Punto de entrada del firmware del robot de minisumo
// Aquí se crean todas las piezas del robot y se conectan entre sí (raíz de
// composición). El modo de funcionamiento lo elige el entorno de PlatformIO:
//   leonardo        -> combate (por defecto)
//   monitoreo       -> combate + telemetría por USB
//   diagnostico     -> lectura de sensores con los motores apagados
//   prueba_motores  -> secuencia para comprobar el sentido de las ruedas
//
// Estructura de carpetas:
//   hardware/    pines, motores, interruptores y módulo de arranque
//   sensores/    lectura y filtrado de los sensores de piso y de enemigo
//   estrategia/  qué hacer según lo que se ve
//   movimiento/  cómo moverse para hacerlo (maniobras y límites de velocidad)
//   robot/       ciclo percibir -> decidir -> actuar
//   modos/       combate, diagnóstico y prueba de motores
#include <Arduino.h>
#include "hardware/DriverMotores.h"
#include "hardware/PlacaXMotion.h"
#include "sensores/Percepcion.h"
#include "estrategia/EstrategiaReglas.h"
#include "estrategia/EstrategiaAprendida.h"
#include "movimiento/ControlMovimiento.h"
#include "robot/ControladorRobot.h"
#include "Parametros.h"

// Elige un tipo u otro en tiempo de compilación (solo se incluye el elegido)
template <bool Condicion, class SiVerdadero, class SiFalso>
struct ElegirTipo { using Tipo = SiVerdadero; };
template <class SiVerdadero, class SiFalso>
struct ElegirTipo<false, SiVerdadero, SiFalso> { using Tipo = SiFalso; };

using EstrategiaElegida = ElegirTipo<UsarPoliticaAprendida, EstrategiaAprendida, EstrategiaReglas>::Tipo;

// --- Piezas del robot ---
PlacaXMotion placa;
DriverMotores motores;
Percepcion percepcion;
EstrategiaElegida estrategia;
ControlMovimiento controlMovimiento;
ControladorRobot robot(percepcion, estrategia, controlMovimiento, motores);

// --- Modo de funcionamiento ---
#if defined(MODO_PRUEBA_MOTORES)
#include "modos/ModoPruebaMotores.h"
ModoPruebaMotores modo(placa, motores);
#elif defined(MODO_DIAGNOSTICO)
#include "modos/ModoDiagnostico.h"
ModoDiagnostico modo(placa, motores, percepcion);
#else
#include "modos/ModoCombate.h"
ModoCombate modo(placa, motores, percepcion, controlMovimiento, robot);
#endif

void setup() {
    modo.iniciar();
}

void loop() {
    modo.actualizar();
}
