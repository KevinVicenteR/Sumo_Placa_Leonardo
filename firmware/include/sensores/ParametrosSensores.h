// ParametrosSensores.h — Sensores de enemigo y de piso
// Lo incluye Parametros.h: los scripts de simulacion/ cambian estos valores
// por su nombre (--param). Velocidades en PWM (0-255), tiempos en ms y
// distancias en metros, salvo que se indique otra cosa.
#ifndef PARAMETROS_SENSORES_H
#define PARAMETROS_SENSORES_H

// --- Sensores de enemigo ---

// Polaridad: 0 = se aprende en modo diagnóstico midiendo sin objetos delante;
// 1 = detectan en HIGH; -1 = detectan en LOW.
constexpr int PolaridadSensoresEnemigo = 0;
// Polaridad usada en combate (nunca se recalibra con el rival delante).
// Un bit por sensor, en orden: lateral izq, 45° izq, central, 45° der, lateral
// der. 31 = los cinco detectan en HIGH.
constexpr int MascaraEnemigosActivosHigh = 31;
// Una detección se mantiene estos ms tras perderla (tolera parpadeos).
constexpr unsigned long TiempoRetencionEnemigo = 40;
// Una detección nueva solo cuenta tras verse estos ms seguidos (0 = al instante).
constexpr unsigned long ConfirmacionSensorEnemigo = 0;

// --- Sensores de piso ---

// El negro se mide al empezar el combate. Se considera línea blanca cuando la
// lectura se aleja del negro más de este porcentaje (y como mínimo
// UmbralLineaMinimo), sin importar si el sensor sube o baja sobre el blanco.
constexpr int PorcentajeUmbralLinea = 15;
constexpr int UmbralLineaMinimo = 60;
constexpr int MuestrasCalibracionPiso = 64;
// Referencias de negro hasta que se calibra (la calibración las sustituye).
constexpr int NegroPisoIzquierdo = 994;
constexpr int NegroPisoDerecho = 972;
// Aviso de "cerca del borde": la lectura se aleja del negro este porcentaje del
// margen de línea. Mientras dura, el avance se limita a VelocidadCercaBorde.
constexpr int PorcentajeCercaBorde = 70;
constexpr int VelocidadCercaBorde = 100;

// --- Protección contra detecciones falsas ---
// Una detección solo termina una acción si dura ConfirmacionDeteccion ms y
// nunca antes del tiempo mínimo de esa acción.
constexpr unsigned long ConfirmacionDeteccion = 5;

#endif
