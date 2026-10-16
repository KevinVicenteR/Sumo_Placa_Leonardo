#include <Arduino.h>
#include "modos/ModoCombate.h"
#include "Pines.h"
#include "Parametros.h"
#if defined(__AVR__)
#include <avr/wdt.h>
#endif

ModoCombate::ModoCombate(PlacaXMotion& placaRef, DriverMotores& motoresRef, Percepcion& percepcionRef,
                         ControlMovimiento& controlRef, ControladorRobot& robotRef)
    : placa(placaRef), motores(motoresRef), percepcion(percepcionRef), controlMovimiento(controlRef),
      robot(robotRef), moduloArranque(FiltroModuloArranqueMs, FiltroParadaModuloMs) {}

void ModoCombate::iniciar() {
#if defined(__AVR__)
    // Tras un reinicio por watchdog, apagarlo para que no vuelva a reiniciar
    MCUSR = 0;
    wdt_disable();
#endif
    placa.inicializarPines();
    motores.deshabilitar();
#if defined(MONITOREO_COMBATE)
    Serial.begin(115200);
#endif
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
#if defined(MONITOREO_COMBATE)
    Serial.print(F("DIP=")); Serial.print(dip); Serial.print(' ');
    Serial.print(F("INICIO negroIzq=")); Serial.print(percepcion.negroIzquierdo());
    Serial.print(F(" margenIzq=")); Serial.print(percepcion.margenIzquierdo());
    Serial.print(F(" negroDer=")); Serial.print(percepcion.negroDerecho());
    Serial.print(F(" margenDer=")); Serial.println(percepcion.margenDerecho());
#endif
}

void ModoCombate::actualizar() {
    // 1. Módulo de arranque: sin RUN, motores apagados y nada más
    if (PIN_MODULO_ARRANQUE >= 0) {
        const bool alto = digitalRead((uint8_t)PIN_MODULO_ARRANQUE) == HIGH;
        if (!moduloArranque.enMarcha(alto == ModuloArranqueActivoAlto, millis())) {
            motores.deshabilitar();
            if (enCombate) {
                inicioParada = millis();
                // Con los motores ya parados se puede escribir en la EEPROM
                cajaNegra.guardar();
            }
            // Parada larga (STOP o PROG del control remoto): reinicia la placa
            // entera, como al encenderla. El siguiente START empieza de cero,
            // vuelve a leer los DIP y hace la rutina de ese modo.
            if (combateEmpezado && ReiniciarTrasParada &&
                millis() - inicioParada >= TiempoReanudarCombate) {
                reiniciarPlaca();
                return;
            }
            if (combateEmpezado) {
                // Sigue leyendo los sensores: si el robot se desliza hasta la
                // línea durante una parada corta, al reanudar lo sabrá (la
                // detección de línea se mantiene aunque el morro quede fuera)
                percepcion.leer();
            }
            enCombate = false;
            return;
        }
        if (!enCombate) {
            // Reanudar solo tras una parada corta de un combate que ya estaba en
            // marcha (interferencia); si no, combate nuevo con su rutina
            const bool paradaCorta = millis() - inicioParada < TiempoReanudarCombate;
            const bool estabaEnMarcha = inicioParada - inicioCombate >= TiempoMinimoParaReanudar;
            if (combateEmpezado && paradaCorta && estabaEnMarcha) {
                // Parada corta (interferencia): sigue el mismo combate
                enCombate = true;
                cajaNegra.anotarInterrupcion();
            } else {
                empezarCombate();
            }
        }
    }

    // 2. Un ciclo de combate (midiendo su duración si hay telemetría)
#if defined(MONITOREO_COMBATE)
    const unsigned long inicioCiclo = micros();
#endif
    robot.actualizar();
    cajaNegra.observar(robot.ultimasLecturas(), controlMovimiento.evasiones(), millis());
#if defined(MONITOREO_COMBATE)
    const unsigned long ciclo = micros() - inicioCiclo;
    if (ciclo > cicloMaximo) cicloMaximo = ciclo;
    enviarTelemetria();
#endif
}

void ModoCombate::reiniciarPlaca() {
    motores.deshabilitar();
#if defined(__AVR__)
    // El watchdog reinicia el microcontrolador en 15 ms: se borra toda la RAM
    // (estado de sensores, maniobras, estrategia...) como al encenderlo
    wdt_enable(WDTO_15MS);
    while (true) {
    }
#else
    // En el simulador: el siguiente RUN empieza un combate nuevo
    combateEmpezado = false;
#endif
}

void ModoCombate::enviarTelemetria() {
#if defined(MONITOREO_COMBATE)
    // Una línea cada 100 ms, solo si cabe en el búfer (no bloquear el combate)
    if (Serial && millis() - ultimaTraza >= 100 && Serial.availableForWrite() >= 60) {
        ultimaTraza = millis();
        Serial.print(F("PISO=")); Serial.print(analogRead(S_PISO_IZQ));
        Serial.print(','); Serial.print(analogRead(S_PISO_DER));
        Serial.print(F(" PWM=")); Serial.print(controlMovimiento.ordenIzquierda());
        Serial.print(','); Serial.print(controlMovimiento.ordenDerecha());
        Serial.print(F(" ENEMIGO_RAW="));
        Serial.print(digitalRead(S_FRONT_IZQ));
        Serial.print(digitalRead(S_FRONT_CEN));
        Serial.print(digitalRead(S_FRONT_DER));
        Serial.print(F(" CICLO_US=")); Serial.println(cicloMaximo);
        cicloMaximo = 0;
    }
#endif
}
