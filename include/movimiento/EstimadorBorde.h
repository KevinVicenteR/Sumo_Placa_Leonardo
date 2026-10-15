// EstimadorBorde.h — Dónde está el robot dentro del dohyo
// Odometría con incertidumbre: integra la velocidad de las ruedas para estimar
// posición (x, y) y rumbo, y lleva la cuenta de cuánto puede haberse
// equivocado. Cada vez que ve la línea corrige la estimación (el robot está
// sobre el borde). Con eso calcula a qué velocidad puede avanzar y aún frenar
// antes del borde en el peor caso.
// Coordenadas: origen en el centro del dohyo. Todo en enteros (sin coma
// flotante, que ocupa mucha Flash en el Leonardo): distancias en 1/65536 m
// (Metro) y ángulos en 1/65536 de vuelta (Vuelta; un int16_t se envuelve solo).
#ifndef ESTIMADOR_BORDE_H
#define ESTIMADOR_BORDE_H

#include <stdint.h>

class EstimadorBorde {
public:
    static constexpr long Metro = 65536;
    static constexpr long Vuelta = 65536;
    // Conversiones de los parámetros (en metros y radianes) al compilar
    static constexpr long metros(float m) { return (long)(m * Metro + (m >= 0 ? 0.5f : -0.5f)); }
    static constexpr long radianes(float r) {
        return (long)(r * (Vuelta / 6.28318531f) + (r >= 0 ? 0.5f : -0.5f));
    }

    EstimadorBorde();

    // Posición de salida conocida: a "radio" del centro con incertidumbre
    // sigmaPos; mirandoAlCentro = el rumbo apunta al centro (con sigmaRumbo)
    void colocarQ(long radio, long sigmaPos, bool mirandoAlCentro, long sigmaRumbo);
    // Igual, con el rumbo conocido: girado "desvio" respecto a mirar al centro
    // (positivo a la izquierda)
    void colocarGiradoQ(long radio, long sigmaPos, long desvio, long sigmaRumbo);

    // Avanza la estimación dtMs milisegundos con estas órdenes de motor.
    // empujon = desplazamiento desconocido (Metro/s) por contacto con el rival
    void predecirMs(int pwmIzq, int pwmDer, unsigned int dtMs, long empujon);

    // Velocidad (Metro/s) que alcanza una rueda con este PWM
    static long velocidadRegimenQ(int pwm);

    // Se ha visto la línea: lado -1 = sensor izq, 1 = der, 0 = los dos
    void lineaVista(int lado, bool avanzando);

    // Distancia (Metro) que puede avanzar en el peor caso antes del margen del borde
    long distanciaLibreQ() const;

    // PWM máximo que aún permite frenar antes del borde
    int limiteAvance() const;

    // La incertidumbre es lo bastante pequeña para fiarse de la predicción
    bool confiable() const;

    // Versiones en metros, segundos y radianes (para los tests y el simulador;
    // el firmware no las usa, así que no enlazan la coma flotante)
    void colocar(float radio, float sigmaPos, bool mirandoAlCentro, float sigmaRumbo) {
        colocarQ(metros(radio), metros(sigmaPos), mirandoAlCentro, radianes(sigmaRumbo));
    }
    void colocarGirado(float radio, float sigmaPos, float desvio, float sigmaRumbo) {
        colocarGiradoQ(metros(radio), metros(sigmaPos), radianes(desvio), radianes(sigmaRumbo));
    }
    void predecir(int pwmIzq, int pwmDer, float dt, float empujon) {
        predecirMs(pwmIzq, pwmDer, (unsigned int)(dt * 1000 + 0.5f), metros(empujon));
    }
    static float velocidadRegimen(int pwm) { return (float)velocidadRegimenQ(pwm) / Metro; }
    float distanciaLibre() const { return (float)distanciaLibreQ() / Metro; }
    float x() const { return (float)px / Metro; }
    float y() const { return (float)py / Metro; }
    float rumbo() const { return th * (6.28318531f / Vuelta); }
    float incertidumbrePosicion() const { return (float)sigPos / Metro; }
    float incertidumbreRumbo() const { return sigTh * (6.28318531f / Vuelta); }

private:
    long px, py;          // posición estimada (Metro)
    int16_t th;           // rumbo estimado (Vuelta)
    long vIzq, vDer;      // velocidad estimada de cada rueda (Metro/s)
    long sigPos, sigTh;   // incertidumbre de posición (Metro) y de rumbo (Vuelta)
};

#endif
