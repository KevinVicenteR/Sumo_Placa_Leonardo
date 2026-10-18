// ParametrosRutinasInicio.h — Rutinas de inicio de siempre (rondas espalda, lado, enfrentados y salida libre)
// Velocidades en PWM (0-255) y tiempos en ms. Lo incluye Parametros.h: los
// scripts de simulacion/ cambian estos valores por su nombre (--param).
#ifndef PARAMETROS_RUTINAS_INICIO_H
#define PARAMETROS_RUTINAS_INICIO_H

// Las usan los modos con EstiloRutina (movimiento/modos/PasoModo.h).
// Ronda 4 (salida libre): gira en el sitio hacia el lado de DIP3 a esta velocidad
// hasta que cualquier sensor vea al rival (entonces gira hacia él y ataca) o
// hasta TiempoMaxGiroRound4 (algo más de una vuelta); después busca como siempre.
// Despacio para no pasar de largo al rival entre dos lecturas de los sensores.
constexpr int VelocidadGiroRound4 = 120;
constexpr unsigned long TiempoMaxGiroRound4 = 1000;
// Rondas 1 y 2: giro inicial.
// Ronda 1 (como en la rama con_control): media vuelta en el sitio a esta velocidad.
constexpr int VelocidadGiroInicio = 180;
// Giro de la ronda 2 (el rival está al lado: girar despacio le deja golpear)
constexpr int VelocidadGiroRound2 = 150;
constexpr unsigned long TiempoMaxGiroEspalda = 500;
constexpr unsigned long TiempoMaxGiroLado = 500;
// Rondas 1 y 2: antes de girar hacia el rival, avanza recto este tiempo a
// VelocidadEsquiva para apartarse de su embestida y atacarlo de lado
// (0 = gira enseguida). En el round 1 separa al robot del rival: están pegados
// y, con las ruedas atrás, al girar las esquinas traseras barren unos 3,6 cm
// hacia atrás, chocan con él y cortan el giro.
constexpr unsigned long TiempoEsquivaRound1 = 0;
constexpr unsigned long TiempoEsquivaRound2 = 0;
constexpr int VelocidadEsquiva = 160;
// Ronda 2: true = pivota sobre una rueda hacia el lado del sensor lateral que ve
// al rival (si lo ve el izquierdo, la rueda izquierda queda parada y empuja la
// derecha), avanzando a la vez contra él; false = gira en el sitio.
constexpr bool PivoteRound2 = true;
constexpr int VelocidadPivoteInicio = 180;
// Ronda 3: avanza despacio y recto hacia delante hasta que cualquier sensor
// (también los laterales) vea al rival (con EsperarRound3, tras
// TiempoAvanceInicio se queda quieto esperándolo). Despacio porque con el
// reparto de peso del robot, al ir rápido o acelerar de golpe se tuerce.
// Por encima de ZonaMuertaPwm para que de verdad avance: con 40 (= la zona
// muerta) casi no se movía. Ronda 3 (11 rivales x 12 factores, sin el de 500 g):
// 40 gana 51,6 % / cae 37,7 %; 60 56,4 / 33,0; 80 57,8 / 31,2; 100 64,9 / 24,3.
constexpr int VelocidadAvanceInicio = 100;
constexpr unsigned long TiempoAvanceInicio = 4000;
// Ronda 3: al ver al rival por delante, acelera de VelocidadAvanceInicio a
// VelocidadEmpuje en este tiempo mientras lo ataca (suave para no torcerse).
constexpr unsigned long TiempoAceleracionRound3 = 2500;
constexpr bool EsperarRound3 = false;
constexpr unsigned long TiempoEsperaRound3 = 3000;
// Rondas 1 y 2: al encontrar al rival embiste durante TiempoEmbestidaInicio,
// subiendo poco a poco de VelocidadEmbestidaInicio a VelocidadEmbestidaMaxima
// mientras lo tenga delante; después mantiene VelocidadAtaqueRound12.
constexpr int VelocidadEmbestidaInicio = 160;
constexpr int VelocidadEmbestidaMaxima = 160;
constexpr int VelocidadAtaqueRound12 = 160;
constexpr unsigned long TiempoEmbestidaInicio = 800;

// Una detección no termina el giro o el avance antes de estos tiempos.
constexpr unsigned long TiempoMinimoGiroEspalda = 80;
constexpr unsigned long TiempoMinimoGiroLado = 60;
constexpr unsigned long TiempoMinimoAvanceInicio = 150;

#endif
