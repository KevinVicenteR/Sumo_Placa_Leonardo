#ifndef CONTROL_MOVIMIENTO_H
#define CONTROL_MOVIMIENTO_H

#include "IControlMovimiento.H"
#include "EstimadorBorde.H"
#include "Parametros.H"

class ControlMovimiento: public IControlMovimiento {
public:
    explicit ControlMovimiento(int rampaPwmPorMs = RampaPwmPorMs, int patronBusqueda = PatronBusqueda)
        : rampa(rampaPwmPorMs), patron(patronBusqueda) {}

    void ejecutar(const DecisionMovimiento& decision, IMotor& motor, unsigned long ahora) override;
    const EstimadorBorde& estimadorBorde() const { return estimador; }
    int limiteAvanceActual() const { return limiteEstimador; }
    int ordenIzquierda() const { return ordenIzq; }
    void iniciarRutina(int rutinaInicio, int ladoInicio) {
        rutina = rutinaInicio;
        ladoRutina = ladoInicio < 0 ? -1 : 1;
        rutinaEnCurso = false;
        conRutina = rutinaInicio != 0;
    }
    int rutinaActual() const { return rutina; }
    void reiniciar();
    int ordenDerecha() const { return ordenDer; }

private:
    enum class Fase { Libre, Retrocediendo, Frenando, Girando, Asentando };

    void mover(IMotor& motor, int izq, int der, bool urgente = false);
    int conRampa(int actual, int objetivo) const;
    void buscarEnArcos(IMotor& motor, unsigned long ahora);
    void iniciarEvasion(int sentido, unsigned long duracionRetroceso,
                        unsigned long duracionGiro, unsigned long ahora, bool unSensor = false);
    bool continuarEvasion(const DecisionMovimiento& decision, IMotor& motor, unsigned long ahora);
    void recordarLadoEnemigo(TipoAccion tipo);
    void girarHaciaLado(IMotor& motor, int sentido);
    void estimarVelocidad(unsigned long ahora);
    void actualizarEstimador(const DecisionMovimiento& decision, unsigned long ahora);
    bool calcularFreno(unsigned long duracionMaxima);
    bool continuarParo(const DecisionMovimiento& decision, IMotor& motor, unsigned long ahora);
    bool continuarGiroLateral(const DecisionMovimiento& decision, IMotor& motor, unsigned long ahora);
    bool continuarRutina(const DecisionMovimiento& decision, IMotor& motor, unsigned long ahora);
    static bool confirmar(bool detectando, unsigned long ahora, bool& activo, unsigned long& inicio);
    void buscar(IMotor& motor, unsigned long ahora);

    Fase fase = Fase::Libre;
    unsigned long inicioFase = 0;
    unsigned long inicioEvasion = 0;
    unsigned long inicioNegro = 0;
    bool negroContinuo = false;
    bool evasionUnSensor = false;
    bool salidaSuaveUnSensor = false;
    unsigned long inicioSalidaSuave = 0;
    int limiteSalidaSuave = 255;
    unsigned long duracionRetroceso = 0;
    unsigned long duracionGiro = 0;
    int sentidoGiro = 1;
    int ordenIzq = 0;
    int ordenDer = 0;
    long estIzq = 0;
    long estDer = 0;
    unsigned long ultimaEstimacion = 0;
    int frenoIzq = 0;
    int frenoDer = 0;
    unsigned long duracionFreno = 0;
    bool veniaAtacando = false;
    bool enParo = false;
    unsigned long inicioParo = 0;
    bool viendoFrente = false;
    unsigned long inicioFrente = 0;
    bool empujando = false;
    bool limitarAvance = false;
    const int rampa;
    const int patron;
    int pasoRampa = 255;
    unsigned long ultimaRampa = 0;
    bool primeraRampa = true;
    unsigned long inicioMovimiento = 0;
    int limiteArranque = 255;
    int limiteEstimador = 255;
    EstimadorBorde estimador;
    unsigned long ultimaPrediccion = 0;
    bool bordeAntes = false;

    int giroLateral = 0;
    unsigned long inicioGiroLateral = 0;

    int rutina = 0;
    int ladoRutina = 1;
    bool rutinaEnCurso = false;
    bool conRutina = false;
    bool embestidaInicio = false;
    bool ataqueDeRutina = false;
    bool frenandoGiroRutina = false;
    unsigned long inicioFrenoRutina = 0;
    unsigned long finEmbestidaInicio = 0;
    unsigned long inicioRutina = 0;
    bool ladoDecidido = false;
    bool detectaRutina = false;
    unsigned long inicioDetectaRutina = 0;
    bool detectaGiroLateral = false;
    unsigned long inicioDetectaGiroLateral = 0;

    int sentidoBusqueda = 1;
    bool buscando = false;
    bool evasionCompleta = false;
    unsigned long inicioBusqueda = 0;
};

#endif
