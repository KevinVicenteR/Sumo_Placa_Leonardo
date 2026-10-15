// ControlPID.h — Regulador PID para centrar al rival mientras se ataca
// Error = posición del rival en los sensores delanteros (-1000 izq .. +1000 der,
// ver DecisionMovimiento::errorRival). Se muestrea cada PeriodoPID ms:
//   P = KpPID · error
//   I = KiPID · suma de (error / 1000) en cada periodo (limitada a ±1)
//   D = KdPID · cambio de (error / 1000) en el periodo, suavizado con
//       FiltroDerivadaPID (los sensores son todo o nada: sin filtro la
//       derivada da golpes que hacen oscilar al robot)
// Corrección = P + I + D, limitada a -1..1 (negativa = girar a la izquierda).
#ifndef CONTROL_PID_H
#define CONTROL_PID_H

class ControlPID {
public:
    // Corrección de este ciclo (-1..1)
    float correccion(int error, unsigned long ahora);
    // Olvida el error anterior y la integral (al dejar de atacar)
    void reiniciar() { activo = false; }

private:
    bool activo = false;
    int errorAnterior = 0;
    unsigned long tAnterior = 0;
    float integral = 0;
    float derivada = 0;
    float salida = 0;
};

#endif
