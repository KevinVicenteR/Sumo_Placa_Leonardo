// Coherencia de los cinco JS40F; ajustable con --param en el simulador.
#ifndef PARAMETROS_DETECCION_H
#define PARAMETROS_DETECCION_H

// false reproduce el filtrado original, sin rechazo de patrones.
// Robot de 500 g, 50 820 combates: gana 81,18 -> 81,55 %, cae 10,69 -> 10,32 %.
constexpr bool RechazarLateralesOpuestos = true;

#endif
