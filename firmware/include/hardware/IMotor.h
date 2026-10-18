// IMotor.h — Interfaz de los motores de tracción
// El resto del programa solo conoce esta interfaz, no los pines del puente H.
// Así los tests pueden usar un motor falso (mock) y el código de movimiento no
// depende del hardware (principio de inversión de dependencias).
#ifndef IMOTOR_H
#define IMOTOR_H

class IMotor {
public:
    virtual ~IMotor() {}
    // Velocidad de cada rueda de -255 (atrás) a 255 (adelante)
    virtual void mover(int velIzq, int velDer) = 0;
    virtual void detener() = 0;
};

#endif
