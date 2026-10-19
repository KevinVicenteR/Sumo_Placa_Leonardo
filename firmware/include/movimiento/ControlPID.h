#ifndef CONTROL_PID_H
#define CONTROL_PID_H

class ControlPID {
public:
    static constexpr int Unidad = 1024;
    int correccion(int error, unsigned long ahora);
    void reiniciar() { activo = false; }

private:
    bool activo = false;
    int errorAnterior;
    unsigned long tAnterior;
    long integral;
    int derivada;
};

#endif
