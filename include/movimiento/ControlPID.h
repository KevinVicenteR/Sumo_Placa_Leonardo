// ControlPID.h — Regulador PD para centrar al rival mientras se ataca
// Error = posición del rival en los sensores delanteros (-1000 izq .. +1000 der,
// ver DecisionMovimiento::errorRival). Corrección = KpPID · error
// + KdPID · (cambio del error por ms) / 1000, limitada a -1..1: es la fracción
// de velocidad que se le quita a la rueda del lado del rival (-1 = rueda
// izquierda parada, gira a la izquierda; +1 = rueda derecha parada).
#ifndef CONTROL_PID_H
#define CONTROL_PID_H

class ControlPID {
public:
    // Corrección de este ciclo (-1..1)
    float correccion(int error, unsigned long ahora);
    // Olvida el error anterior (al dejar de atacar)
    void reiniciar() { activo = false; }

    float derivadaActual() const { return derivada; }

private:
    bool activo = false;
    int errorAnterior = 0;
    unsigned long tAnterior = 0;
    float derivada = 0;   // cambio del error por ms, en milésimas (-1..1 por ms)
};

#endif
