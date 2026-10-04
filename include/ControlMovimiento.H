#ifndef CONTROL_MOVIMIENTO_H
#define CONTROL_MOVIMIENTO_H

#include "IControlMovimiento.H"

// Ejecuta decisiones sin bloquear. La evasión de borde es una maniobra
// temporizada (freno de cada rueda -> retroceso -> freno -> giro) que avanza
// un paso en cada llamada.
// La búsqueda alterna un avance corto con un giro en el sitio hacia el
// lado donde se vio al enemigo por última vez, y una pausa para asentarse.
// El ataque se acerca a VelocidadAtaque y empuja a VelocidadEmpuje una vez
// encima del enemigo; con el enemigo pegado de frente no se suelta en el borde.
class ControlMovimiento: public IControlMovimiento {
public:
    void ejecutar(const DecisionMovimiento& decision, IMotor& motor, unsigned long ahora) override;

private:
    enum class Fase { Libre, FrenandoRuedas, Retrocediendo, Frenando, Girando };

    // Envía la orden al motor y la recuerda para estimar la velocidad de las ruedas
    void mover(IMotor& motor, int izq, int der);
    // sentido: +1 gira a la derecha (horario), -1 a la izquierda
    void iniciarEvasion(int sentido, unsigned long duracionRetroceso,
                        unsigned long duracionGiro, unsigned long ahora);
    bool continuarEvasion(const DecisionMovimiento& decision, IMotor& motor, unsigned long ahora);
    void recordarLadoEnemigo(TipoAccion tipo);
    void girarHaciaLado(IMotor& motor, int sentido);
    void estimarVelocidad(unsigned long ahora);
    bool calcularFreno(unsigned long duracionMaxima);
    bool continuarParo(const DecisionMovimiento& decision, IMotor& motor, unsigned long ahora);
    bool continuarGiroLateral(const DecisionMovimiento& decision, IMotor& motor, unsigned long ahora);
    bool resistirEnBorde(const DecisionMovimiento& decision, IMotor& motor);
    void buscar(IMotor& motor, unsigned long ahora);

    Fase fase = Fase::Libre;
    unsigned long inicioFase = 0;
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
    bool evasionEnSitio = false;
    bool viendoFrente = false;       // está atacando (enemigo visto por delante)
    unsigned long inicioFrente = 0;  // desde cuándo, sin interrupción
    bool empujando = false;          // ataca desde hace TiempoEmbestida o más

    int giroLateral = 0;  // sentido del giro comprometido hacia un enemigo visto de lado; 0 si no
    unsigned long inicioGiroLateral = 0;

    int sentidoBusqueda = 1;
    bool buscando = false;
    bool evasionCompleta = false;
    unsigned long inicioBusqueda = 0;
};

#endif
