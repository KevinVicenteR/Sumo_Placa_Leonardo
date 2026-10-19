#include <Arduino.h>
#include "modos/ModoCombate.h"
#include "Pines.h"
#include "Parametros.h"
#if defined(__AVR__)
#include <avr/wdt.h>
#include <EEPROM.h>

namespace {
// CajaNegra ocupa 0..147 en AVR (cabecera de 4 y 8 registros de 18).
// Byte 1023 reservado al aviso de reinicio; Caterina puede borrar MCUSR.
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
    // La marca sobrevive al bootloader; consumirla solo una vez.
    reiniciadaPorParada = EEPROM.read(DireccionAvisoReinicio) == MarcaAvisoReinicio;
    EEPROM.update(DireccionAvisoReinicio, 0);
    if (AvisoListoLed) {
        TX_RX_LED_INIT;
    }
#endif
    placa.inicializarPines();
    motores.deshabilitar();
    // Sin módulo de arranque, el combate empieza nada más encender
    if (PIN_MODULO_ARRANQUE < 0) {
        empezarCombate();
    }
}

void ModoCombate::empezarCombate() {
    // Reinicio completo: el robot queda como recién encendido, sin nada del
    // combate anterior (sensores, maniobras, rutina), y en el modo de los DIP
    placa.inicializarPines();
    motores.deshabilitar();
    avisoListo(false);
    percepcion = Percepcion();
    // Medir el negro con los motores apagados. Los sensores de enemigo NO se
    // calibran: el rival puede estar delante al empezar.
    percepcion.calibrarPiso();
    controlMovimiento.reiniciar();
    // Rutina de inicio según los DIP (DIP3 elige el lado del giro)
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
    // 1. Módulo de arranque: sin RUN, motores apagados y nada más
    if (PIN_MODULO_ARRANQUE >= 0) {
        const bool alto = digitalRead((uint8_t)PIN_MODULO_ARRANQUE) == HIGH;
        const bool run = alto == ModuloArranqueActivoAlto;
        const bool marcha = moduloArranque.enMarcha(run, millis());
        if (!marcha) {
            motores.deshabilitar();
            if (combateEmpezado) {
                // Toda parada confirmada reinicia, sin consultar sensores.
                reiniciarPlaca();
                return;
            }
            actualizarAvisoListo();
            return;
        }
        // Durante el filtro STOP tampoco se ejecuta la estrategia.
        if (!run) {
            motores.deshabilitar();
            return;
        }
        if (!enCombate) {
            empezarCombate();
        }
    }

    // 2. Un ciclo de combate
    robot.actualizar();
    cajaNegra.observar(robot.ultimasLecturas(), controlMovimiento.evasiones(), millis());
}

void ModoCombate::reiniciarPlaca() {
    // Para todo: motores apagados y aviso apagado hasta que vuelva a estar lista
    motores.deshabilitar();
    avisoListo(false);
    cajaNegra.guardar();  // no vuelve a escribir si ya estaba guardada
#if defined(__AVR__)
    EEPROM.update(DireccionAvisoReinicio, MarcaAvisoReinicio);
    // El watchdog reinicia el microcontrolador en 15 ms: se borra toda la RAM
    // (estado de sensores, maniobras, estrategia...) como al encenderlo
    wdt_enable(WDTO_15MS);
    while (true) {
    }
#else
    // En el simulador: volver a esperar STOP estable y un START nuevo
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
    // Tras un reinicio por STOP o PROG parpadea ParpadeosListo veces; después,
    // encendido fijo hasta el START
    const unsigned long t = millis();
    const bool parpadeando = reiniciadaPorParada && t < 2UL * ParpadeosListo * PeriodoParpadeoListo;
    avisoListo(!parpadeando || (t / PeriodoParpadeoListo) % 2 == 0);
}
