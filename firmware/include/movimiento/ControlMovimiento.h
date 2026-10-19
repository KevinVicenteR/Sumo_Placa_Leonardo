#ifndef CONTROL_MOVIMIENTO_H
#define CONTROL_MOVIMIENTO_H

#include "movimiento/IControlMovimiento.h"
#include "movimiento/MandoMotores.h"
#include "maniobras/ManiobraEvasion.h"
#include "modos_rumania/RutinaInicio.h"
#include "maniobras/BusquedaRival.h"
#include "maniobras/GiroLateral.h"
#include "maniobras/ParoPerdida.h"
#include "maniobras/ControlAtaque.h"
#include "borde/LimitadorBorde.h"
#include "configuracion/Parametros.h"
#include "redes/ControlRedModo4.h"

class ControlMovimiento: public IControlMovimiento {
public:
    explicit ControlMovimiento(int rampaPwmPorMs = RampaPwmPorMs, int patronBusqueda = PatronBusqueda)
        : rampa(rampaPwmPorMs), patron(patronBusqueda), mando(rampaPwmPorMs), busqueda(patronBusqueda) {}

    void ejecutar(const DecisionMovimiento& decision, IMotor& motor, unsigned long ahora) override;

    void reiniciar();
    void iniciarRutina(int rutinaInicio, int ladoInicio) {
        competencia = rutinaInicio;
        ++generacion;
        controlRed.reiniciar();
        evasion.configurarModo4(mejorasActivas());
        rutina.iniciar(rutinaInicio, ladoInicio);
        limitadorBorde.colocarSalida(rutinaInicio);
    }

    void habilitarRedes() { redesHabilitadas = true; }
    unsigned char modoCompetencia() const { return competencia; }
    bool esModo4() const { return competencia == 4; }
    bool mejorasActivas() const {
        return redesHabilitadas && (competencia == 4 ||
               (competencia >= 1 && competencia <= 3 && rutina.actual() == 0));
    }
    unsigned char generacionCombate() const { return generacion; }

    int rutinaActual() const { return rutina.actual(); }
    int ordenIzquierda() const { return mando.ordenIzquierda(); }
    int ordenDerecha() const { return mando.ordenDerecha(); }
    int velocidadAvanceEstimada() const { return mando.velocidadAvanceEstimada(); }
    unsigned int evasiones() const { return numEvasiones; }
    const EstimadorBorde& estimadorBorde() const { return limitadorBorde.estimador(); }
    int limiteAvanceActual() const { return limitadorBorde.limite(); }

private:
    __attribute__((noinline)) void actualizarLimites(const DecisionMovimiento& decision);
    void empezarEvasionSiHayBorde(const DecisionMovimiento& decision, unsigned long ahora);
    bool resistirEnBorde(const DecisionMovimiento& decision, IMotor& motor);
    __attribute__((noinline)) void ejecutarAccion(const DecisionMovimiento& decision, IMotor& motor, unsigned long ahora);

    unsigned char competencia = 0;
    bool redesHabilitadas = false, mejorasEsteCiclo = false;
    unsigned char generacion = 0;
    ControlRedModo4 controlRed;
    int rampa;
    int patron;

    MandoMotores mando;
    ManiobraEvasion evasion;
    RutinaInicio rutina;
    BusquedaRival busqueda;
    GiroLateral giroLateral;
    ParoPerdida paro;
    ControlAtaque ataque;
    LimitadorBorde limitadorBorde;
    unsigned int numEvasiones = 0;
};

#endif
