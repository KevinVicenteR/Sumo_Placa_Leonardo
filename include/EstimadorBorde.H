#ifndef ESTIMADOR_BORDE_H
#define ESTIMADOR_BORDE_H

class EstimadorBorde {
public:
    EstimadorBorde();

    void predecir(int pwmIzq, int pwmDer, float dt, float empujon);

    static float velocidadRegimen(int pwm);

    void lineaVista(int lado, bool avanzando);

    float distanciaLibre() const;

    int limiteAvance() const;

    bool confiable() const;

    float x() const { return px; }
    float y() const { return py; }
    float rumbo() const { return th; }
    float incertidumbrePosicion() const { return sigPos; }
    float incertidumbreRumbo() const { return sigTh; }

private:
    float px, py, th;
    float vIzq, vDer;
    float sigPos, sigTh;
};

#endif
