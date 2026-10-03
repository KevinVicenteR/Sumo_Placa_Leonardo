#ifndef PARAMETROS_H
#define PARAMETROS_H

// Referencia física: motores de 750 rpm (~1.2 m/s en vacío con rueda de ~3 cm),
// dohyo circular de 70 cm de diámetro con borde blanco de 1 cm.
// Con un borde tan delgado el sensor apenas lo ve ~8 ms a velocidad máxima y el
// robot sigue avanzando varios cm mientras frena, por eso la búsqueda va más lenta.

// Velocidades PWM (0-255)
constexpr int VelocidadMaxima = 255;
constexpr int VelocidadCurva = 150;
constexpr int VelocidadRetroceso = 255;
constexpr int VelocidadAvance = 130;
constexpr int VelocidadBusquedaDer = 110;
constexpr int VelocidadPivoteLateral = 90;

// Sensores de piso: una lectura analógica menor a este valor es línea blanca
constexpr int UmbralLinea = 50;

// Tiempos (ms)
constexpr unsigned long TiempoInicioReglamentario = 5000;
// Incluye el tiempo para frenar la inercia antes de empezar a retroceder
constexpr unsigned long TiempoRetroceso = 200;
constexpr unsigned long TiempoRetrocesoAmbos = 300;
// Giro de ~120-150° a máxima velocidad; 350 ms daba más de media vuelta
constexpr unsigned long TiempoGiroEvasion = 200;
constexpr unsigned long TiempoRetencionEnemigo = 40;

#endif
