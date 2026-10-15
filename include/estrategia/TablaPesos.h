// TablaPesos.h — Pesos y sesgos de la red de EstrategiaPesos
// Cada fila es una neurona (una acción de redpesos::Acciones). Cada columna es
// el peso de una entrada (1 = sí, 0 = no):
//   Frontal, 45° izq, 45° der, lateral izq, lateral der: el sensor ve al rival
//   Lado: último lado donde se vio (1 = der)
//   Borde: un sensor de piso empieza a ver claro (cerca de la línea)
//   Perd: no ve al rival, pero lo vio hace menos de RecuerdoRivalMs
//   Emp: el frontal lo ve sin parar desde hace ContactoFrontalMs (empujando)
// Activación de una neurona = f(suma de entrada * peso + sesgo), con f la
// función elegida en FuncionActivacion (Parametros.h). Gana la más activada.
// Los valores de partida reproducen exactamente EstrategiaReglas: se pueden
// retocar a mano para cambiar las prioridades de los movimientos.
#ifndef TABLA_PESOS_H
#define TABLA_PESOS_H

#include <stdint.h>

namespace redpesos {

constexpr int NumEntradas = 9;
constexpr int NumNeuronas = 8;

constexpr int8_t Pesos[NumNeuronas][NumEntradas] = {
    //  Fr  45I  45D  LatI LatD Lado Borde Perd Emp
    {   10,   1,   1,   0,   0,   0,   0,   0,   0 },   // AtaqueFrontal
    {   10,   3,  -3,   0,   0,   0,   0,   0,   0 },   // AjusteIzq
    {   10,  -3,   3,   0,   0,   0,   0,   0,   0 },   // AjusteDer
    {  -10,   8,   0,   0,   0,   0,   0,   0,   0 },   // CorregirIzq
    {  -10,  -1,   8,   0,   0,   0,   0,   0,   0 },   // CorregirDer
    {  -10, -10, -10,   4,   0,   0,   0,   0,   0 },   // DefensaIzq
    {  -10, -10, -10,  -1,   4,   0,   0,   0,   0 },   // DefensaDer
    {  -10, -10, -10, -10, -10,   0,   0,   0,   0 },   // Busqueda
};

constexpr int8_t Sesgos[NumNeuronas] = {
    -5,   // AtaqueFrontal
    -6,   // AjusteIzq
    -6,   // AjusteDer
    -2,   // CorregirIzq
    -2,   // CorregirDer
    -1,   // DefensaIzq
    -1,   // DefensaDer
     1,   // Busqueda: la única activa cuando no se ve nada
};

}

#endif
