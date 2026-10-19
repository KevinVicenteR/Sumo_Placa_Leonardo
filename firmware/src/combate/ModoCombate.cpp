#include <Arduino.h>
#include "combate/ModoCombate.h"
#include "configuracion/Pines.h"
#include "configuracion/Parametros.h"
#if defined(__AVR__)
#include <avr/wdt.h>
#include <EEPROM.h>

namespace {
constexpr int DireccionAvisoReinicio = 1023;
constexpr uint8_t MarcaAvisoReinicio = 0xA7;
}
#endif

ModoCombate::ModoCombate(PlacaXMotion& placaRef, DriverMotores& motoresRef, Percepcion& percepcionRef,
                         ControlMovimiento& controlRef, ControladorRobot& robotRef)
    : placa(placaRef), motores(motoresRef), percepcion(percepcionRef), controlMovimiento(controlRef),
      robot(robotRef), moduloArranque(FiltroModuloArranqueMs, FiltroParadaModuloMs, StopMinimoAntesDeStart) {}

void ModoCombate::iniciar() {
#if defined(__AVR__)
    MCUSR = 0;
    wdt_disable();
    reiniciadaPorParada = EEPROM.read(DireccionAvisoReinicio) == MarcaAvisoReinicio;
    EEPROM.update(DireccionAvisoReinicio, 0);
    if (AvisoListoLed) {
        TX_RX_LED_INIT;
    }
#endif
    placa.inicializarPines();
    motores.deshabilitar();
    if (PIN_MODULO_ARRANQUE < 0) {
        empezarCombate();
    }
}

void ModoCombate::empezarCombate() {
    placa.inicializarPines();
    motores.deshabilitar();
    avisoListo(false);
    percepcion = Percepcion();
    percepcion.calibrarPiso();
    controlMovimiento.reiniciar();
    const int dip = placa.leerInterruptores();
    const int rutina = RutinaForzada >= 0 ? RutinaForzada : rutinaSegunInterruptores(dip);
    const int lado = LadoForzado != 0 ? LadoForzado : (dip & 4) ? -1 : 1;
    controlMovimiento.iniciarRutina(rutina, lado);
    cajaNegra.empezar(dip, percepcion.negroIzquierdo(), percepcion.negroDerecho(), millis());
    enCombate = true;
    combateEmpezado = true;
    inicioCombate = millis();
}

void ModoCombate::actualizar() {
    if (PIN_MODULO_ARRANQUE >= 0) {
        const bool alto = digitalRead((uint8_t)PIN_MODULO_ARRANQUE) == HIGH;
        const bool run = alto == ModuloArranqueActivoAlto;
        const bool marcha = moduloArranque.enMarcha(run, millis());
        if (!marcha) {
            motores.deshabilitar();
            if (combateEmpezado) {
                reiniciarPlaca();
                return;
            }
            actualizarAvisoListo();
            return;
        }
        if (!run) {
            motores.deshabilitar();
            return;
        }
        if (!enCombate) {
            empezarCombate();
        }
    }

    robot.actualizar();
    cajaNegra.observar(robot.ultimasLecturas(), controlMovimiento.evasiones(), millis());
}

void ModoCombate::reiniciarPlaca() {
    motores.deshabilitar();
    avisoListo(false);
    cajaNegra.guardar();
#if defined(__AVR__)
    EEPROM.update(DireccionAvisoReinicio, MarcaAvisoReinicio);
    wdt_enable(WDTO_15MS);
    while (true) {
    }
#else
    combateEmpezado = false;
    enCombate = false;
    moduloArranque = ModuloArranque(FiltroModuloArranqueMs, FiltroParadaModuloMs,
                                    StopMinimoAntesDeStart);
#endif
}

void ModoCombate::avisoListo(bool encendido) {
#if defined(__AVR__)
    if (!AvisoListoLed) {
        return;
    }
    if (encendido) {
        TXLED1;
        RXLED1;
    } else {
        TXLED0;
        RXLED0;
    }
#else
    (void)encendido;
#endif
}

void ModoCombate::actualizarAvisoListo() {
    const unsigned long t = millis();
    const bool parpadeando = reiniciadaPorParada && t < 2UL * ParpadeosListo * PeriodoParpadeoListo;
    avisoListo(!parpadeando || (t / PeriodoParpadeoListo) % 2 == 0);
}
