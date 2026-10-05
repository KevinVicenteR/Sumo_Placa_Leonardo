// La página web compila el firmware sin biblioteca estándar: lo que el firmware
// usa de <stdlib.h> (abs) ya lo define el Arduino.h simulado de esta carpeta.
#ifndef SIMULACION_WEB_STDLIB_H
#define SIMULACION_WEB_STDLIB_H
#include "Arduino.h"
#endif
