#ifndef CONTROL_MOVIMIENTO_H
#define CONTROL_MOVIMIENTO_H

#include "IControlMovimiento.H"

// Ejecuta decisiones sin bloquear. La evasión de borde es una maniobra
// temporizada (freno de cada rueda -> retroceso -> freno -> giro) que avanza
// un paso en cada llamada.
// La búsqueda alterna un avance corto con un giro en el sitio hacia el
// lado donde se vio al enemigo por última vez.
class ControlMovimiento: public IControlMovimiento {
public:
    void ejecutar(const DecisionMovimiento& decision, IMotor& motor, unsigned long ahora) override;

private:
    enum class Fase { Libre, FrenandoRuedas, Retrocediendo, Frenando, Girando };

    // Envía la orden al motor y recuerda si es un giro en el sitio
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
    int sentidoEnSitio = 0;  // sentido del último giro en el sitio ordenado; 0 si no gira
    bool evasionEnSitio = false;
    bool huboAvance = false;          // false al arrancar: el robot está quieto
    unsigned long ultimoAvance = 0;   // última orden de avance o retroceso
    unsigned long ahoraActual = 0;

    int giroLateral = 0;  // sentido del giro comprometido hacia un enemigo visto de lado; 0 si no
    unsigned long inicioGiroLateral = 0;

    int sentidoBusqueda = 1;
    bool buscando = false;
    bool evasionCompleta = false;
    unsigned long inicioBusqueda = 0;
};

#endif
