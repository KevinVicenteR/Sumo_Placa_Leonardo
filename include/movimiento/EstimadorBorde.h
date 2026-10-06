// =============================================================================
// EstimadorBorde.h — Dónde está el robot dentro del dohyo
// -----------------------------------------------------------------------------
// Odometría con incertidumbre: integra la velocidad de las ruedas para estimar
// posición (x, y) y rumbo, y lleva la cuenta de cuánto puede haberse
// equivocado. Cada vez que ve la línea corrige la estimación (el robot está
// sobre el borde). Con eso calcula a qué velocidad puede avanzar y aún frenar
// antes del borde en el peor caso.
// Coordenadas: origen en el centro del dohyo, metros y radianes.
// =============================================================================
#ifndef ESTIMADOR_BORDE_H
#define ESTIMADOR_BORDE_H

class EstimadorBorde {
public:
    EstimadorBorde();

    // Avanza la estimación dt segundos con estas órdenes de motor.
    // empujon = desplazamiento desconocido (m/s) por contacto con el rival
    void predecir(int pwmIzq, int pwmDer, float dt, float empujon);

    // Velocidad (m/s) que alcanza una rueda con este PWM
    static float velocidadRegimen(int pwm);

    // Se ha visto la línea: lado -1 = sensor izq, 1 = der, 0 = los dos
    void lineaVista(int lado, bool avanzando);

    // Distancia (m) que puede avanzar en el peor caso antes del margen del borde
    float distanciaLibre() const;

    // PWM máximo que aún permite frenar antes del borde
    int limiteAvance() const;

    // La incertidumbre es lo bastante pequeña para fiarse de la predicción
    bool confiable() const;

    float x() const { return px; }
    float y() const { return py; }
    float rumbo() const { return th; }
    float incertidumbrePosicion() const { return sigPos; }
    float incertidumbreRumbo() const { return sigTh; }

private:
    float px, py, th;     // posición y rumbo estimados
    float vIzq, vDer;     // velocidad estimada de cada rueda (m/s)
    float sigPos, sigTh;  // incertidumbre de posición (m) y de rumbo (rad)
};

#endif
