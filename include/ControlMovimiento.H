#ifndef CONTROL_MOVIMIENTO_H
#define CONTROL_MOVIMIENTO_H

#include "IControlMovimiento.H"
#include "EstimadorBorde.H"
#include "Parametros.H"

// Ejecuta decisiones sin bloquear. La evasión de borde es una maniobra
// temporizada (retroceso recto -> paro -> giro lento -> paro) que avanza
// un paso en cada llamada.
// La búsqueda alterna un avance corto con un giro en el sitio hacia el
// lado donde se vio al enemigo por última vez, y una pausa para asentarse.
// El ataque se acerca a VelocidadAtaque y empuja a VelocidadEmpuje una vez
// encima del enemigo; el borde tiene prioridad sobre el ataque.
class ControlMovimiento: public IControlMovimiento {
public:
    // Por defecto con los movimientos de Parametros.H; los tests del
    // comportamiento clásico usan (0, 0, false): sin rampa y búsqueda con giro.
    // El tercer argumento se conserva por compatibilidad; la evasión usa retroceso y giro.
    explicit ControlMovimiento(int rampaPwmPorMs = RampaPwmPorMs, int patronBusqueda = PatronBusqueda,
                               bool escapeEnArco = EscapeEnArco)
        : rampa(rampaPwmPorMs), patron(patronBusqueda), escapeArco(escapeEnArco) {}

    void ejecutar(const DecisionMovimiento& decision, IMotor& motor, unsigned long ahora) override;
    const EstimadorBorde& estimadorBorde() const { return estimador; }
    int limiteAvanceActual() const { return limiteEstimador; }
    int ordenIzquierda() const { return ordenIzq; }
    // Rutina de inicio del round (ver Parametros.H): 0 ninguna, 1 espalda con
    // espalda, 2 lado a lado, 3 enfrentados lejos. lado: +1 derecha, -1 izquierda.
    // Empieza en la primera llamada a ejecutar().
    void iniciarRutina(int rutinaInicio, int ladoInicio) {
        rutina = rutinaInicio;
        ladoRutina = ladoInicio < 0 ? -1 : 1;
        rutinaEnCurso = false;
        conRutina = rutinaInicio != 0;
    }
    int rutinaActual() const { return rutina; }
    // Vuelve al estado del encendido (para cada START del módulo de arranque)
    void reiniciar();
    int ordenDerecha() const { return ordenDer; }

private:
    enum class Fase { Libre, Retrocediendo, Frenando, Girando, Asentando };

    // Envía la orden al motor y la recuerda para estimar la velocidad de las ruedas
    // urgente: sin rampa (frenar, escapar del borde, paro)
    void mover(IMotor& motor, int izq, int der, bool urgente = false);
    int conRampa(int actual, int objetivo) const;
    void buscarEnArcos(IMotor& motor, unsigned long ahora);
    // sentido: +1 gira a la derecha (horario), -1 a la izquierda
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
    bool resistirEnBorde(const DecisionMovimiento& decision, IMotor& motor);
    bool continuarRutina(const DecisionMovimiento& decision, IMotor& motor, unsigned long ahora);
    // true si 'detectando' lleva ConfirmacionDeteccion ms seguidos
    static bool confirmar(bool detectando, unsigned long ahora, bool& activo, unsigned long& inicio);
    void buscar(IMotor& motor, unsigned long ahora);
    void avanzarPorPulsos(IMotor& motor, unsigned long t);

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
    int ordenIzq = 0;  // última orden enviada a cada motor
    int ordenDer = 0;
    long estIzq = 0;   // velocidad estimada de cada rueda (PWM × 256)
    long estDer = 0;
    unsigned long ultimaEstimacion = 0;
    int frenoIzq = 0;
    int frenoDer = 0;
    unsigned long duracionFreno = 0;
    bool veniaAtacando = false;
    bool enParo = false;
    unsigned long inicioParo = 0;
    bool viendoFrente = false;       // está atacando (enemigo visto por delante)
    unsigned long inicioFrente = 0;  // desde cuándo, sin interrupción
    bool empujando = false;          // ataca desde hace TiempoEmbestida o más
    bool limitarAvance = false;      // cerca del borde: avance y ataque limitados
    const int rampa;
    const int patron;
    const bool escapeArco;
    int pasoRampa = 255;             // lo que puede subir la orden en este ciclo
    unsigned long ultimaRampa = 0;
    bool primeraRampa = true;
    unsigned long inicioMovimiento = 0;
    int limiteArranque = 255;
    int limiteEstimador = 255;       // PWM máximo hacia delante según el borde previsto
    EstimadorBorde estimador;
    unsigned long ultimaPrediccion = 0;
    bool bordeAntes = false;         // en el ciclo anterior ya se veía la línea

    int giroLateral = 0;  // sentido del giro comprometido hacia un enemigo visto de lado; 0 si no
    unsigned long inicioGiroLateral = 0;

    int rutina = 0;
    int ladoRutina = 1;
    bool rutinaEnCurso = false;
    bool conRutina = false;           // se eligió una rutina: sin arranque suave
    bool embestidaInicio = false;     // la rutina encontró al rival pegado
    bool ataqueDeRutina = false;      // sigue el ataque que empezó la rutina del round 1 o 2
    bool frenandoGiroRutina = false;  // contragiro para no pasarse de largo
    unsigned long inicioFrenoRutina = 0;
    unsigned long finEmbestidaInicio = 0;
    unsigned long inicioRutina = 0;
    bool ladoDecidido = false;        // round 2: un sensor lateral ya eligió el lado
    bool detectaRutina = false;       // confirmación de la detección que termina la rutina
    unsigned long inicioDetectaRutina = 0;
    bool detectaGiroLateral = false;
    unsigned long inicioDetectaGiroLateral = 0;

    int sentidoBusqueda = 1;
    bool buscando = false;
    bool evasionCompleta = false;
    unsigned long inicioBusqueda = 0;
};

#endif
