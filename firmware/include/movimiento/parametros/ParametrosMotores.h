// ParametrosMotores.h — Mando de los motores: límites, rampa y giros sobre una llanta
// Velocidades en PWM (0-255) y tiempos en ms. Lo incluye Parametros.h: los
// scripts de simulacion/ cambian estos valores por su nombre (--param).
#ifndef PARAMETROS_MOTORES_H
#define PARAMETROS_MOTORES_H

// Límite absoluto que se manda a cada motor.
constexpr int VelocidadMaxima = 255;

// --- Giros ---
// true = todos los giros en el sitio (una rueda adelante y otra atrás) se hacen
// pivotando sobre una llanta: la interior se para y la exterior va a la mayor
// de las dos velocidades. Los frenos y la evasión del borde no pivotan (ver
// PivoteEnEvasion en ParametrosEvasion.h).
constexpr bool GirarSobreUnaRueda = true;

// Sin rutina de inicio, todo avance se limita durante este tiempo al empezar.
constexpr unsigned long TiempoArranqueSuave = 1000;
// Rampa: cuánto puede subir la orden de cada motor por ms (0 = sin rampa).
// Bajar la velocidad, frenar y escapar del borde son siempre inmediatos.
constexpr int RampaPwmPorMs = 1;

#endif
