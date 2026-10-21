#include <Arduino.h>
#include "modos/ModoCombate.h"
#include "Pines.h"
#include "Parametros.h"
#if defined(__AVR__)
#define DECODE_HASH
#include <IRremote.hpp>
#include <avr/wdt.h>
#include <EEPROM.h>

namespace {
// CajaNegra ocupa 0..147 en AVR (cabecera de 4 y 8 registros de 18).
// Byte 1023 reservado al aviso de reinicio; Caterina puede borrar MCUSR.
constexpr int DireccionAvisoReinicio = 1023;
constexpr uint8_t MarcaAvisoReinicio = 0xA7;
constexpr int DireccionCodigosIR = 200;
constexpr uint16_t MarcaCodigosIR = 0x1838;
constexpr unsigned long TiempoLiberarControlIRMs = 300;

struct DatosCodigosIR {
    uint16_t marca;
    CodigoControlIR iniciar;
    CodigoControlIR detener;
    CodigoControlIR reiniciar;
};
}
#endif

ModoCombate::ModoCombate(PlacaXMotion& placaRef, DriverMotores& motoresRef, Percepcion& percepcionRef,
                         ControlMovimiento& controlRef, ControladorRobot& robotRef)
    : placa(placaRef), motores(motoresRef), percepcion(percepcionRef), controlMovimiento(controlRef),
      robot(robotRef), moduloArranque(FiltroModuloArranqueMs, FiltroParadaModuloMs,
                                      StopMinimoAntesDeStart) {}

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
#if defined(__AVR__)
    Serial.begin(115200);
    if (PIN_MODULO_ARRANQUE >= 0) {
        IrReceiver.begin(static_cast<uint8_t>(PIN_MODULO_ARRANQUE), DISABLE_LED_FEEDBACK);
    }
    if (PIN_MODULO_ARRANQUE >= 0 && leerCodigoIR(codigoIniciar)) {
        controlIRBloqueado = reiniciadaPorParada;
        ultimoPaqueteIR = millis();
        mensajeInicialIR = false;
    } else if (PIN_MODULO_ARRANQUE >= 0) {
        iniciarAprendizajeIR();
    }
#endif
    // Sin receptor infrarrojo, el combate empieza nada más encender.
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
#if defined(MONITOREO_COMBATE)
    // DIP y negro calibrado de los sensores de piso (N=izq,der)
    Serial.print(F("DIP=")); Serial.print(dip);
    Serial.print(F(" N=")); Serial.print(percepcion.negroIzquierdo());
    Serial.print(','); Serial.println(percepcion.negroDerecho());
#endif
}

void ModoCombate::actualizar() {
#if defined(__AVR__)
    procesarControlIR();
    if (aprendizajeIR) {
        motores.deshabilitar();
        const unsigned long ahora = millis();
        if (Serial && ahora - ultimoAvisoAprendizaje >= 2000) {
            ultimoAvisoAprendizaje = ahora;
            Serial.println(codigosAprendidos == 0 ? F("IR: pulsa el boton START") :
                           codigosAprendidos == 1 ? F("IR: pulsa el boton STOP") :
                           F("IR: pulsa el boton RESET"));
        }
        return;
    }
#else
    // La simulacion conserva la entrada digital RUN/STOP para sus escenarios.
    if (PIN_MODULO_ARRANQUE >= 0) {
        const bool alto = digitalRead(static_cast<uint8_t>(PIN_MODULO_ARRANQUE)) == HIGH;
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
        if (!enCombate) empezarCombate();
    }
#endif

#if defined(__AVR__)
    if (PIN_MODULO_ARRANQUE >= 0 && !combateEmpezado) {
        motores.deshabilitar();
        actualizarAvisoListo();
        return;
    }
#endif

#if defined(__AVR__)
    // Espera reglamentaria tras el START: quieto, con los motores apagados.
    if (millis() - inicioCombate < EsperaInicioCombate) {
        motores.deshabilitar();
        return;
    }
#endif

    // 2. Un ciclo de combate
    robot.actualizar();
    cajaNegra.observar(robot.ultimasLecturas(), controlMovimiento.evasiones(), millis());
#if defined(MONITOREO_COMBATE)
    enviarTelemetria();
#endif
}

void ModoCombate::iniciarAprendizajeIR() {
    motores.deshabilitar();
    if (combateEmpezado) cajaNegra.guardar();
    enCombate = false;
    combateEmpezado = false;
    aprendizajeIR = true;
    codigosAprendidos = 0;
    controlIRBloqueado = false;
    ultimoAvisoAprendizaje = 0;
    avisoListo(true);
#if defined(__AVR__)
    Serial.println(F("Aprendizaje IR: pulsa START, luego STOP y finalmente RESET."));
#endif
}

void ModoCombate::imprimirCodigoIR(const CodigoControlIR& codigo) {
#if defined(__AVR__)
    Serial.print(F(" protocolo=0x")); Serial.print(codigo.protocolo, HEX);
    Serial.print(F(" direccion=0x")); Serial.print(codigo.direccion, HEX);
    Serial.print(F(" comando=0x")); Serial.print(codigo.comando, HEX);
    Serial.print(F(" hash=0x")); Serial.println(codigo.hash, HEX);
#else
    (void)codigo;
#endif
}

bool ModoCombate::leerCodigoIR(CodigoControlIR& codigo) {
#if defined(__AVR__)
    DatosCodigosIR datos{};
    EEPROM.get(DireccionCodigosIR, datos);
    if (datos.marca != MarcaCodigosIR || datos.iniciar.protocolo == 0 ||
        datos.detener.protocolo == 0 || datos.reiniciar.protocolo == 0) {
        return false;
    }
    codigoIniciar = datos.iniciar;
    codigoDetener = datos.detener;
    codigoReiniciar = datos.reiniciar;
    codigo = datos.iniciar;
    return true;
#else
    (void)codigo;
    return false;
#endif
}

void ModoCombate::guardarCodigosIR() {
#if defined(__AVR__)
    const DatosCodigosIR datos{MarcaCodigosIR, codigoIniciar, codigoDetener, codigoReiniciar};
    EEPROM.put(DireccionCodigosIR, datos);
#endif
}

void ModoCombate::detenerCombate() {
    motores.deshabilitar();
    if (combateEmpezado) cajaNegra.guardar();
    enCombate = false;
    combateEmpezado = false;
    avisoListo(true);
#if defined(__AVR__)
    Serial.println(F("STOP: motores apagados; esperando START."));
#endif
}

void ModoCombate::procesarControlIR() {
#if defined(__AVR__)
    if (PIN_MODULO_ARRANQUE < 0) return;

    if (Serial.available()) {
        const char orden = static_cast<char>(Serial.read());
        if (orden == 'l' || orden == 'L') {
            iniciarAprendizajeIR();
            return;
        }
    }

    if (IrReceiver.decode()) {
        const auto datos = IrReceiver.decodedIRData;
        ultimoPaqueteIR = millis();
        const bool repeticion = (datos.flags & IRDATA_FLAGS_IS_REPEAT) != 0;
        IrReceiver.resume();
        if (repeticion) return;

        const CodigoControlIR recibido{
            static_cast<uint8_t>(datos.protocol == UNKNOWN ? 0xFF : datos.protocol),
            datos.protocol == UNKNOWN ? 0 : datos.address,
            datos.protocol == UNKNOWN ? 0 : datos.command,
            datos.protocol == UNKNOWN ? datos.decodedRawData : 0
        };
        if (recibido.protocolo == 0 ||
            (recibido.protocolo == 0xFF && recibido.hash == 0)) {
            return;
        }

        if (aprendizajeIR) {
            if ((codigosAprendidos > 0 && mismoCodigoIR(recibido, codigoIniciar)) ||
                (codigosAprendidos > 1 && mismoCodigoIR(recibido, codigoDetener))) {
                return;
            }
            if (codigosAprendidos == 0) {
                codigoIniciar = recibido;
                Serial.print(F("START aprendido:")); imprimirCodigoIR(recibido);
                codigosAprendidos = 1;
            } else if (codigosAprendidos == 1) {
                codigoDetener = recibido;
                Serial.print(F("STOP aprendido:")); imprimirCodigoIR(recibido);
                codigosAprendidos = 2;
            } else {
                codigoReiniciar = recibido;
                Serial.print(F("RESET aprendido:")); imprimirCodigoIR(recibido);
                guardarCodigosIR();
                aprendizajeIR = false;
                mensajeInicialIR = false;
                Serial.println(F("Codigos guardados. Motores apagados; pulsa START para iniciar."));
            }
            controlIRBloqueado = true;
            return;
        }

        if (controlIRBloqueado) return;
        const AccionControlIR accion = buscarAccionIR(
            recibido, codigoIniciar, codigoDetener, codigoReiniciar);
        if (accion == AccionControlIR::Ninguna) return;
        controlIRBloqueado = true;

        if (accion == AccionControlIR::Iniciar) {
            if (!combateEmpezado) empezarCombate();
        } else if (accion == AccionControlIR::Detener) {
            detenerCombate();
        } else if (accion == AccionControlIR::Reiniciar) {
            motores.deshabilitar();
            Serial.println(F("RESET: reiniciando placa."));
            reiniciarPlaca();
        }
        return;
    }

    if (controlIRBloqueado && millis() - ultimoPaqueteIR >= TiempoLiberarControlIRMs) {
        controlIRBloqueado = false;
    }
#endif
}

void ModoCombate::reiniciarPlaca() {
    // Para todo hasta que la placa vuelva a estar lista
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
    // En el simulador: volver a esperar STOP estable y un START nuevo.
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

void ModoCombate::enviarTelemetria() {
#if defined(MONITOREO_COMBATE)
    // Una línea cada 100 ms, solo si cabe en el búfer (no bloquear el combate):
    // PISO=izq,der  PWM=orden de motores izq,der  ENE=sensores 45I,F,45D
    if (Serial && millis() - ultimaTraza >= 100 && Serial.availableForWrite() >= 60) {
        ultimaTraza = millis();
        Serial.print(F("P=")); Serial.print(analogRead(S_PISO_IZQ));
        Serial.print(','); Serial.print(analogRead(S_PISO_DER));
        Serial.print(F(" M=")); Serial.print(controlMovimiento.ordenIzquierda());
        Serial.print(','); Serial.print(controlMovimiento.ordenDerecha());
        Serial.print(F(" ENE="));
        Serial.print(digitalRead(S_FRONT_IZQ));
        Serial.print(digitalRead(S_FRONT_CEN));
        Serial.println(digitalRead(S_FRONT_DER));
    }
#endif
}
