// ParametrosMotores.h — Mando de los motores: límites, rampa y giros sobre una llanta
// Velocidades en PWM (0-255) y tiempos en ms. Lo incluye Parametros.h: los
// scripts de simulacion/ cambian estos valores por su nombre (--param).
#ifndef PARAMETROS_MOTORES_H
#define PARAMETROS_MOTORES_H
#define CABLEADO_MOTORES_X3

// Sentido ajustado al cableado de los motores de 400 rpm del Sumo X3.
constexpr bool InvertirMotorIzq = true;
constexpr bool InvertirMotorDer = true;

// Límite absoluto que se manda a cada motor.
constexpr int VelocidadMaxima = 255;

// --- Giros ---
// true = todos los giros en el sitio (una rueda adelante y otra atrás) se hacen
// pivotando sobre una llanta: la interior se para y la exterior va a la mayor
// de las dos velocidades. Los frenos y la evasión del borde no pivotan (ver
// PivoteEnEvasion en ParametrosEvasion.h).
constexpr bool GirarSobreUnaRueda = true;
// true = solo pivota en el modo 4; los modos 1-3 giran en el sitio (como secihti)
constexpr bool PivoteSoloModo4 = true;

// Ajuste fino de cada motor (% de la orden), para que avance recto: los dos
// motores no empujan igual. Si al avanzar se desvía a la derecha, la rueda
// izquierda empuja más: baja PorcentajeMotorIzquierdo (y al revés). Solo en el
// robot real; el simulador modela los dos motores iguales.
// X3: 100/100; el ajuste del 92 % correspondía al otro robot.
constexpr int PorcentajeMotorIzquierdo = 100;
constexpr int PorcentajeMotorDerecho = 100;

// Sin rutina de inicio, todo avance se limita durante este tiempo al empezar.
constexpr unsigned long TiempoArranqueSuave = 1000;
// Rampa: cuánto puede subir la orden de cada motor por ms (0 = sin rampa).
// Bajar la velocidad, frenar y escapar del borde son siempre inmediatos.
constexpr int RampaPwmPorMs = 1;

#endif
