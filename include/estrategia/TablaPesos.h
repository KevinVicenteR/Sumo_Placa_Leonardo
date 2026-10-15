// TablaPesos.h — Pesos y sesgos de la red de EstrategiaPesos
// Cada fila es una neurona (una acción de redpesos::Acciones). Cada columna es
// el peso de una entrada (1 = el sensor ve al rival, 0 = no):
//   Frontal, 45° izq, 45° der, lateral izq, lateral der, último lado (1 = der)
// Activación de una neurona = f(suma de entrada * peso + sesgo), con f la
// función elegida en FuncionActivacion (Parametros.h). Gana la más activada.
// Ajustados con simulacion/optimizar_pesos.py partiendo de unos pesos que
// reproducían EstrategiaReglas. Se pueden retocar a mano para cambiar las
// prioridades de los movimientos.
#ifndef TABLA_PESOS_H
#define TABLA_PESOS_H

#include <stdint.h>

namespace redpesos {

constexpr int NumEntradas = 6;
constexpr int NumNeuronas = 8;

constexpr int8_t Pesos[NumNeuronas][NumEntradas] = {
    //  Fr  45I  45D  LatI LatD Lado
    {   10,   1,   1,   1,   0,  -4 },   // AtaqueFrontal
    {   10,   3,  -3,   0,   0,   0 },   // AjusteIzq
    {   10,  -3,   3,   0,   0,   0 },   // AjusteDer
    {  -10,   4,   0,  -4,  -4,  -4 },   // CorregirIzq
    {  -10,  -5,   8,  -4,  -4,  -4 },   // CorregirDer
    {  -10, -10, -10,   4,  -4,  -4 },   // DefensaIzq
    {  -10, -10, -10,   3,   4,   4 },   // DefensaDer
    {  -10, -10, -10, -10, -10,   0 },   // Busqueda
};

constexpr int8_t Sesgos[NumNeuronas] = {
    -5,   // AtaqueFrontal
    -6,   // AjusteIzq
    -6,   // AjusteDer
    -2,   // CorregirIzq
    -2,   // CorregirDer
     3,   // DefensaIzq
    -1,   // DefensaDer
     1,   // Busqueda: la única activa cuando no se ve nada
};

}

#endif
