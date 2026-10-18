// ParametrosAtaque.h — Ataque: velocidades, embestida y corrección hacia el rival
// Velocidades en PWM (0-255) y tiempos en ms. Lo incluye Parametros.h: los
// scripts de simulacion/ cambian estos valores por su nombre (--param).
#ifndef PARAMETROS_ATAQUE_H
#define PARAMETROS_ATAQUE_H

// Velocidad al empezar a atacar; sube poco a poco hasta VelocidadEmpuje
// durante TiempoEmbestida mientras tenga al rival de frente.
constexpr int VelocidadAtaque = 130;
constexpr int VelocidadEmpuje = 255;
// Rival en los tres sensores delanteros a la vez: es un robot cercano (un reflejo
// no ocupa tres sensores), así que ataca ya a esta velocidad (0 = sin efecto).
constexpr int VelocidadAtaqueDirecto = 0;
// Rival de frente y también en un sensor de 45°: la rueda de ese lado va a
// este porcentaje de la otra para centrarlo sin dejar de empujar.
constexpr int PorcentajeAjuste = 85;
// Rival visto solo por un sensor de 45°: true = la rueda de ese lado se para y
// hace de pivote; false = curva con las dos ruedas (la interior a VelocidadCurva).
constexpr bool Corregir45EnPivote = true;
constexpr int VelocidadCurva = 85;
// true = al llegar a la línea empujando al rival, sigue empujando en vez de
// retroceder (solo tras TiempoEmbestida empujándolo de frente).
constexpr bool ResistirEnBorde = false;

// Tiempo de la subida de VelocidadAtaque a VelocidadEmpuje. Vuelve a empezar
// si pierde al rival de frente o evade el borde.
constexpr unsigned long TiempoEmbestida = 2500;

// Si la curva fuera más rápida que el ataque, al corregir hacia un rival visto
// a 45° la rueda interior iría más rápida y el robot giraría al revés.
static_assert(VelocidadCurva <= VelocidadAtaque, "VelocidadCurva debe ser <= VelocidadAtaque");

// Rondas 1 y 2: al encontrar al rival embiste durante TiempoEmbestidaInicio,
// subiendo poco a poco de VelocidadEmbestidaInicio a VelocidadEmbestidaMaxima
// mientras lo tenga delante; después mantiene VelocidadAtaqueRound12.
constexpr int VelocidadEmbestidaInicio = 160;
constexpr int VelocidadEmbestidaMaxima = 160;
constexpr int VelocidadAtaqueRound12 = 160;
constexpr unsigned long TiempoEmbestidaInicio = 800;

// Ataque a fondo de las rondas 3 y libre sin llegar a empujar durante este
// tiempo: pasa al ataque normal (respeta el limitador de borde).
constexpr unsigned long TiempoMaxAtaqueLibre = 150;

#endif
