// Parametros.h — Valores ajustables del Sumo X3 (motores de 400 rpm)
// Cada grupo está en el archivo de la parte del robot que controla:
//   sensores/ParametrosSensores.h               sensores de enemigo y de piso
//   hardware/ParametrosArranque.h               módulo de START, espera de 5 s y DIP
//   movimiento/parametros/ParametrosMotores.h   límites, rampa, pivote y ajuste de motores
//   movimiento/parametros/ParametrosAtaque.h    ataque, embestida y corrección hacia el rival
//   movimiento/parametros/ParametrosGiroLateral.h  giro hacia un rival visto de lado
//   movimiento/parametros/ParametrosEvasion.h   evasión del borde
//   movimiento/parametros/ParametrosBusqueda.h  búsqueda del rival
//   movimiento/parametros/ParametrosFrenado.h   frenado al perder al rival
//   movimiento/parametros/ParametrosEstimadorBorde.h  estimador de borde
//   movimiento/parametros/ParametrosPID.h       PID de la red de control
//   estrategia/ParametrosEstrategia.h           política aprendida y redes neuronales
//   movimiento/rondas/Ronda1.h ... Libre.h      cada ronda (modo de los DIP)
// Valores elegidos con el simulador; hay que comprobarlos en el robot real. Los
// scripts de simulacion/ cambian estos valores por su nombre (--param): si se
// renombra uno, hay que actualizar los scripts.
#ifndef PARAMETROS_H
#define PARAMETROS_H

#include "sensores/ParametrosSensores.h"
#include "hardware/ParametrosArranque.h"
#include "movimiento/parametros/ParametrosEstimadorBorde.h"
#include "movimiento/parametros/ParametrosMotores.h"
#include "movimiento/parametros/ParametrosAtaque.h"
#include "movimiento/parametros/ParametrosGiroLateral.h"
#include "movimiento/parametros/ParametrosEvasion.h"
#include "movimiento/parametros/ParametrosBusqueda.h"
#include "movimiento/parametros/ParametrosFrenado.h"
#include "movimiento/parametros/ParametrosPID.h"
#include "estrategia/ParametrosEstrategia.h"

#endif
