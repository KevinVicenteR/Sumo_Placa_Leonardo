// RedPesos.h — Red de una capa: entradas * pesos + sesgo -> activación
// Cada acción de movimiento es una neurona con sus pesos y su sesgo
// (TablaPesos.h). En cada ciclo se calcula la suma ponderada de las entradas,
// se pasa por la función de activación y gana la neurona más activada. Si
// ninguna llega a UmbralActivacion, el robot busca.
#ifndef RED_PESOS_H
#define RED_PESOS_H

#include "estrategia/DecisionMovimiento.h"
#include "estrategia/TablaPesos.h"
#include "sensores/LecturasSensores.h"

namespace redpesos {

// Funciones de activación (valor de FuncionActivacion en Parametros.h)
constexpr int Sigmoide = 0;   // 0..1, suave
constexpr int Relu = 1;       // 0 si es negativa, igual si es positiva
constexpr int Tanh = 2;       // -1..1, suave

// Acción de cada neurona (mismo orden que las filas de Pesos)
constexpr TipoAccion Acciones[NumNeuronas] = {
    TipoAccion::AtaqueFrontal, TipoAccion::AjusteIzq,  TipoAccion::AjusteDer,
    TipoAccion::CorregirIzq,   TipoAccion::CorregirDer, TipoAccion::DefensaIzq,
    TipoAccion::DefensaDer,    TipoAccion::Busqueda,
};
constexpr int NeuronaBusqueda = NumNeuronas - 1;

// Lecturas + último lado visto (0 izq, 1 der) -> vector de entradas
void entradas(const LecturasSensores& lecturas, int ladoUltimo, int8_t x[NumEntradas]);

// Suma ponderada de una neurona: sesgo + sum(x * peso)
int sumaPonderada(int neurona, const int8_t x[NumEntradas]);

// Función de activación aplicada a una suma ponderada. Las curvas suaves usan
// x / (1 + |x|), que tiene la misma forma que la sigmoide y la tangente
// hiperbólica sin necesitar exp() (lento en el Leonardo y sin libm en la web).
float activar(int suma, int funcion);

// Neurona ganadora (índice en Acciones). Rellena activaciones[] si no es nulo.
int elegirNeurona(const int8_t x[NumEntradas], int funcion, float umbral,
                  float* activaciones = nullptr);

}

#endif
