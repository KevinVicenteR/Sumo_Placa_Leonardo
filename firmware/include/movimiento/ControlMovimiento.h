// ControlMovimiento.h — Convierte cada decisión en órdenes de motor
// Coordina las maniobras (patrón Mediador): cada una vive en su propia clase y
// esta decide, por orden de prioridad, cuál manda en cada ciclo:
//   1. ManiobraEvasion  — alejarse del borde.
//   2. RutinaInicio     — primer movimiento del round.
//   3. ParoPerdida      — frenar al perder al rival en pleno ataque.
//   4. GiroLateral      — girar hacia un rival visto de lado.
//   5. Acción normal    — atacar (ControlAtaque) o buscar (BusquedaRival).
// Todas mueven los motores a través de MandoMotores, que aplica los límites
// de velocidad y la rampa.
#ifndef CONTROL_MOVIMIENTO_H
#define CONTROL_MOVIMIENTO_H

#include "movimiento/IControlMovimiento.h"
#include "movimiento/MandoMotores.h"
#include "movimiento/ManiobraEvasion.h"
#include "movimiento/RutinaInicio.h"
#include "movimiento/BusquedaRival.h"
#include "movimiento/GiroLateral.h"
#include "movimiento/ParoPerdida.h"
#include "movimiento/ControlAtaque.h"
#include "movimiento/LimitadorBorde.h"
#include "Parametros.h"
#include "Parametros.h"
#include "movimiento/ControlRedModo4.h"

class ControlMovimiento: public IControlMovimiento {
public:
    explicit ControlMovimiento(int rampaPwmPorMs = RampaPwmPorMs, int patronBusqueda = PatronBusqueda)
        : rampa(rampaPwmPorMs), patron(patronBusqueda), mando(rampaPwmPorMs), busqueda(patronBusqueda) {}

    void ejecutar(const DecisionMovimiento& decision, IMotor& motor, unsigned long ahora) override;

    // Vuelve al estado inicial (antes de un combate nuevo)
    void reiniciar();
    // Rutina del round (ver RutinaInicio) y lado del giro (1 der, -1 izq)
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
    // Las salidas 1–3 usan la ruta histórica hasta que termina su maniobra.
    bool mejorasActivas() const {
        return redesHabilitadas && (competencia == 4 ||
               (MejorasEnModos1a3 && competencia >= 1 && competencia <= 3 && rutina.actual() == 0));
    }
    unsigned char generacionCombate() const { return generacion; }

    // --- Consultas (telemetría, tests y simulador) ---
    int rutinaActual() const { return rutina.actual(); }
    int ordenIzquierda() const { return mando.ordenIzquierda(); }
    int ordenDerecha() const { return mando.ordenDerecha(); }
    int velocidadAvanceEstimada() const { return mando.velocidadAvanceEstimada(); }
    // Evasiones del borde desde el último reiniciar() (para la caja negra)
    unsigned int evasiones() const { return numEvasiones; }
    const EstimadorBorde& estimadorBorde() const { return limitadorBorde.estimador(); }
    int limiteAvanceActual() const { return limitadorBorde.limite(); }

private:
    // Pasa a MandoMotores los límites de avance actuales de cada maniobra
    __attribute__((noinline)) void actualizarLimites(const DecisionMovimiento& decision);
    // Empieza la evasión si la estrategia ve el borde
    void empezarEvasionSiHayBorde(const DecisionMovimiento& decision, unsigned long ahora);
    // Opción ResistirEnBorde: seguir empujando al rival al llegar a la línea
    bool resistirEnBorde(const DecisionMovimiento& decision, IMotor& motor);
    // Acción normal: atacar, girar hacia un lado o buscar
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
