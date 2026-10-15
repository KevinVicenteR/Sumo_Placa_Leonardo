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

// Lecturas + último lado visto (0 izq, 1 der) + rival recién perdido + frontal
// sostenido -> vector de entradas
void entradas(const LecturasSensores& lecturas, int ladoUltimo, int8_t x[NumEntradas],
              bool recienPerdido = false, bool frontalSostenido = false);

// Suma ponderada de una neurona: sesgo + sum(x * peso)
int sumaPonderada(int neurona, const int8_t x[NumEntradas]);

// Función de activación aplicada a una suma ponderada. Las curvas suaves usan
// x / (1 + |x|), que tiene la misma forma que la sigmoide y la tangente
// hiperbólica sin necesitar exp() (lento en el Leonardo y sin libm en la web).
constexpr float suave(int z) { return z / (1.0f + (z < 0 ? -z : z)); }   // -1..1
constexpr float activar(int suma, int funcion) {
    return funcion == Relu ? (suma > 0 ? (float)suma : 0.0f)
         : funcion == Tanh ? suave(suma)
         : 0.5f * (1.0f + suave(suma));
}

// Las tres activaciones crecen con la suma: gana la neurona de mayor suma y
// "activación >= umbral" equivale a "suma >= sumaMinima". Así, en el robot, la
// red solo hace cuentas con enteros (sumaMinima se calcula al compilar).
constexpr int buscarSumaMinima(int desde, int hasta, int funcion, float umbral) {
    return desde >= hasta ? desde
         : activar(desde + (hasta - desde) / 2, funcion) >= umbral
             ? buscarSumaMinima(desde, desde + (hasta - desde) / 2, funcion, umbral)
             : buscarSumaMinima(desde + (hasta - desde) / 2 + 1, hasta, funcion, umbral);
}
constexpr int sumaMinima(int funcion, float umbral) {
    return buscarSumaMinima(-4096, 4096, funcion, umbral);
}

// Neurona ganadora (índice en Acciones): la de mayor suma, si llega a minima;
// si no, Busqueda. Rellena sumas[] si no es nulo.
int elegirPorSuma(const int8_t x[NumEntradas], int minima, int16_t* sumas = nullptr);

// Igual, con la función de activación y el umbral dados en tiempo de ejecución
inline int elegirNeurona(const int8_t x[NumEntradas], int funcion, float umbral) {
    return elegirPorSuma(x, sumaMinima(funcion, umbral));
}

}

#endif
