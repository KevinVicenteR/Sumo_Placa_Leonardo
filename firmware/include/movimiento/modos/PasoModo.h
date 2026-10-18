// PasoModo.h — Qué hace cada modo (DIP) y lo común a los estilos de arranque
// Los DIP eligen el modo (DIP1 = bit 0, DIP2 = bit 1); DIP3 elige el lado del
// rival o del giro (off = derecha, ON = izquierda). Cada modo hace una rutina
// de inicio de siempre (movimiento/RutinaInicio) o un estilo de arranque de
// esta carpeta, cada uno con sus velocidades, sus tiempos y su lógica:
//   ModoDefensivo.h    quieto encarándolo; lo recibe con la pala y empuja
//   ModoContragolpe.h  quieto hasta verlo y ataca a fondo; si no aparece, avanza lento
//   ModoLado.h         de lado: pivota hacia el rival (derecha o izquierda) y ataca
//   ModoAgresivo.h     avance recto y giro a fondo hacia el rival
//   ModoArco.h         斜め / 回り込み: arco a fondo hacia DIP3 y giro hacia dentro
// Los estilos solo dirigen el arranque. Durante su TiempoMinimo hacen siempre su
// maniobra sin mirar los sensores de rival (solo la línea del borde la
// interrumpe); después, en cuanto el frontal ve al rival (confirmado) o se agota
// su tiempo, el combate sigue con la estrategia normal.
#ifndef PASO_MODO_H
#define PASO_MODO_H

#include "movimiento/MandoMotores.h"
#include "estrategia/DecisionMovimiento.h"

// Estilos de arranque. EstiloRutina = la rutina de inicio de siempre de ese
// modo (1 espalda con espalda, 2 lado a lado, 3 enfrentados, 4 salida libre).
constexpr int EstiloRutina = 0, EstiloDefensivo = 1, EstiloContragolpe = 2, EstiloLado = 3,
              EstiloAgresivo = 4, EstiloArco = 5;
// pesos_neuronales_rutina3v2 (rounds de SECIHTI):
//   DIP1 DIP2  modo
//   off  off   1: espalda con espalda -> rutina de siempre (media vuelta)
//   ON   off   2: lado a lado -> rutina de siempre (pivote hacia el rival)
//   off  ON    3: enfrentados -> contragolpe (ModoContragolpe.h)
//   ON   ON    4: salida libre -> arco (ModoArco.h)
constexpr int EstiloModo1 = EstiloRutina;
constexpr int EstiloModo2 = EstiloRutina;
constexpr int EstiloModo3 = EstiloContragolpe;
constexpr int EstiloModo4 = EstiloArco;
constexpr int estiloDeModo(int modo) {
    return modo == 1 ? EstiloModo1 : modo == 2 ? EstiloModo2 : modo == 3 ? EstiloModo3
         : modo == 4 ? EstiloModo4 : EstiloRutina;
}
// Algún modo usa este estilo (los que no se usan no ocupan Flash)
constexpr bool usaEstilo(int e) {
    return EstiloModo1 == e || EstiloModo2 == e || EstiloModo3 == e || EstiloModo4 == e;
}

// Lo que un modo necesita saber en cada ciclo
struct PasoModo {
    const DecisionMovimiento& decision;
    MandoMotores& mando;
    IMotor& motor;
    unsigned long ahora;
    unsigned long t;     // ms desde que empezó el modo
    int lado;            // DIP3: 1 = derecha, -1 = izquierda
    int visto;           // sensor de 45° o lateral que lo ve: 1 der, -1 izq, 0 ninguno
    bool deFrente;       // el frontal lo ve (confirmado)
};

// Qué hace el modo tras este ciclo
enum class ResultadoModo : unsigned char {
    Sigue,      // sigue el modo (ya ha mandado los motores)
    Ataca,      // termina con ataque a fondo (ControlAtaque, VelocidadEmpuje)
    Termina     // termina sin más: sigue la estrategia normal
};

#endif
