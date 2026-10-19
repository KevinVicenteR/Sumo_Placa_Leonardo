#ifndef PARAMETROS_ATAQUE_H
#define PARAMETROS_ATAQUE_H

constexpr int VelocidadAtaque = 130;
constexpr int VelocidadEmpuje = 255;
constexpr int VelocidadAtaqueDirecto = 0;
constexpr int PorcentajeAjuste = 85;
constexpr bool Corregir45EnPivote = true;
constexpr int VelocidadCurva = 85;
constexpr bool ResistirEnBorde = false;

constexpr unsigned long TiempoEmbestida = 2500;

static_assert(VelocidadCurva <= VelocidadAtaque, "VelocidadCurva debe ser <= VelocidadAtaque");

#endif
