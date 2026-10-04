#ifndef PARAMETROS_H
#define PARAMETROS_H

// Velocidades PWM (0-255)
constexpr int VelocidadMaxima = 255;
constexpr int VelocidadCurva = 40;
constexpr int VelocidadRetroceso = 255;
constexpr int VelocidadAvance = 160;
constexpr int VelocidadBusquedaDer = 140;
constexpr int VelocidadPivoteLateral = 90;

// Sensores de piso: al arrancar se mide el negro del dohyo y se considera
// línea blanca cualquier lectura por debajo de este porcentaje de ese valor.
constexpr int PorcentajeUmbralLinea = 50;
constexpr int MuestrasCalibracionPiso = 64;

// Tiempos (ms)
constexpr unsigned long TiempoInicioReglamentario = 5000;
constexpr unsigned long TiempoRetroceso = 200;
constexpr unsigned long TiempoRetrocesoAmbos = 300;
constexpr unsigned long TiempoGiroEvasion = 350;
constexpr unsigned long TiempoRetencionEnemigo = 40;

#endif
