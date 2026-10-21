// ModoCombate.h — Funcionamiento normal en competencia
// Aprende y decodifica tres comandos IR; START inicia, STOP apaga y RESET
// reinicia la placa. Mientras espera START, los motores permanecen apagados.
#ifndef MODO_COMBATE_H
#define MODO_COMBATE_H

#include "hardware/DriverMotores.h"
#include "hardware/PlacaXMotion.h"
#include "hardware/ModuloArranque.h"
#include "hardware/ComandosControlIR.h"
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
    // Reinicia el microcontrolador tras recibir el comando RESET.
    void reiniciarPlaca();
    void detenerCombate();
    void iniciarAprendizajeIR();
    void procesarControlIR();
    void imprimirCodigoIR(const CodigoControlIR& codigo);
    bool leerCodigoIR(CodigoControlIR& codigo);
    void guardarCodigosIR();
    void enviarTelemetria();
    // LED RX y TX: listo mientras espera el START
    void avisoListo(bool encendido);
    void actualizarAvisoListo();

    PlacaXMotion& placa;
    DriverMotores& motores;
    Percepcion& percepcion;
    ControlMovimiento& controlMovimiento;
    ControladorRobot& robot;
    ModuloArranque moduloArranque;
    CajaNegra cajaNegra;
    CodigoControlIR codigoIniciar;
    CodigoControlIR codigoDetener;
    CodigoControlIR codigoReiniciar;
    bool enCombate = false;
    // También cuenta el START durante la espera reglamentaria.
    bool combateEmpezado = false;
    bool aprendizajeIR = false;
    bool controlIRBloqueado = false;
    bool mensajeInicialIR = false;
    uint8_t codigosAprendidos = 0;
    unsigned long ultimoPaqueteIR = 0;
    unsigned long ultimoAvisoAprendizaje = 0;
    unsigned long inicioCombate = 0;
    // Reinicio por RESET: parpadea el aviso de listo.
    bool reiniciadaPorParada = false;
#if defined(MONITOREO_COMBATE)
    unsigned long ultimaTraza = 0;
#endif
};

#endif
