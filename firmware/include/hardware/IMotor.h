#ifndef IMOTOR_H
#define IMOTOR_H

class IMotor {
public:
    virtual ~IMotor() {}
    virtual void mover(int velIzq, int velDer) = 0;
    virtual void detener() = 0;
};

#endif
