#ifndef PARAMETROS_H
#define PARAMETROS_H

// Velocidades PWM (0-255)
constexpr int VelocidadMaxima = 255;
constexpr int VelocidadCurva = 150;
constexpr int VelocidadRetroceso = 255;
constexpr int VelocidadAvance = 160;
constexpr int VelocidadBusquedaDer = 140;
constexpr int VelocidadPivoteLateral = 90;

// Sensores de piso: una lectura analógica menor a este valor es línea blanca
constexpr int UmbralLinea = 50;

// Tiempos (ms)
constexpr unsigned long TiempoInicioReglamentario = 5000;
constexpr unsigned long TiempoRetroceso = 150;
constexpr unsigned long TiempoRetrocesoAmbos = 250;
constexpr unsigned long TiempoGiroEvasion = 350;
constexpr unsigned long TiempoRetencionEnemigo = 40;

#endif
