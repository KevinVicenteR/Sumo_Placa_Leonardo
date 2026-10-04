#ifndef ESTIMADOR_BORDE_H
#define ESTIMADOR_BORDE_H

// Estima dónde está el robot dentro del dohyo para prever el borde antes de que
// lo vean los sensores de piso.
//
// Sin encoders ni giroscopio, la posición se predice integrando la velocidad
// estimada de cada rueda (la orden filtrada con la inercia del robot), y la
// incertidumbre de esa predicción crece con lo recorrido, con lo girado y con
// los choques. Cada vez que un sensor de piso ve la línea llega una medida: el
// robot está a un radio conocido del centro, mirando hacia fuera hacia el lado
// del sensor que la tocó. Esa medida se combina con la predicción como en un
// filtro de Kalman (promedio ponderado por la inversa de las varianzas), y la
// incertidumbre vuelve a ser pequeña.
//
// Como el dohyo es circular solo importan la distancia al centro y el rumbo
// respecto al radio, así que el sistema de referencia se puede fijar donde se
// quiera; al arrancar se toma el centro, con la incertidumbre de la posición de
// salida y el rumbo desconocido.
//
// Con la posición y su incertidumbre se calcula, en el peor caso, cuánto puede
// avanzar el robot antes de que su centro llegue al borde, y de ahí la
// velocidad máxima con la que aún puede frenar a tiempo.
class EstimadorBorde {
public:
    EstimadorBorde();

    // Avanza la predicción dt segundos con las órdenes PWM de cada motor
    // (-255..255). empujon: m/s de desplazamiento desconocido que puede estar
    // causando el enemigo (0 si no hay contacto).
    void predecir(int pwmIzq, int pwmDer, float dt, float empujon);

    // Velocidad de régimen (m/s) de una rueda con esta orden, según el modelo
    // identificado en la simulación (zona muerta y recta hasta la máxima)
    static float velocidadRegimen(int pwm);

    // Un sensor de piso acaba de ver la línea. lado: -1 el izquierdo, +1 el
    // derecho, 0 los dos. avanzando: el robot iba hacia delante.
    void lineaVista(int lado, bool avanzando);

    // Distancia (m) que puede recorrer hacia delante, en el peor caso, antes de
    // que su centro llegue al borde menos el margen de seguridad
    float distanciaLibre() const;

    // PWM máximo hacia delante con el que aún puede frenar antes del borde
    int limiteAvance() const;

    // La incertidumbre es lo bastante pequeña como para fiarse de la predicción
    // (en la práctica, poco después de ver la línea)
    bool confiable() const;

    float x() const { return px; }
    float y() const { return py; }
    float rumbo() const { return th; }
    float incertidumbrePosicion() const { return sigPos; }
    float incertidumbreRumbo() const { return sigTh; }

private:
    float px, py, th;     // posición (m) respecto al centro y rumbo (rad)
    float vIzq, vDer;     // velocidad predicha de cada rueda (m/s)
    float sigPos, sigTh;  // desviación típica de la posición (m) y del rumbo (rad)
};

#endif
