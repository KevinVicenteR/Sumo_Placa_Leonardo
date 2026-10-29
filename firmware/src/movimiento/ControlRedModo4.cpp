#include "movimiento/ControlRedModo4.h"
#include "movimiento/RedControl.h"
#include "ParametrosModo4.h"
namespace {
int limitar(int x, int minimo, int maximo) { return x < minimo ? minimo : x > maximo ? maximo : x; }
}
void ControlRedModo4::ejecutar(const DecisionMovimiento& d, MandoMotores& mando, IMotor& motor,
                              unsigned long ahora, int velocidad, bool empujando, int limiteBorde, int modo) {
    const int error = limitar(d.errorRival, -1000, 1000);
    int izq, der;
    if (esAtaque(d.tipo)) {
        const int interior = Corregir45EnPivote ? 0 : VelocidadCurva * velocidad / VelocidadAtaque;
        izq = der = velocidad;
        if (d.tipo == TipoAccion::AjusteIzq) izq = velocidad * PorcentajeAjuste / 100;
        if (d.tipo == TipoAccion::AjusteDer) der = velocidad * PorcentajeAjuste / 100;
        if (d.tipo == TipoAccion::CorregirIzq) izq = interior;
        if (d.tipo == TipoAccion::CorregirDer) der = interior;
    } else {
        izq = d.tipo == TipoAccion::EsquivarIzq ? VelocidadEsquivaInterior : VelocidadEsquivaExterior;
        der = d.tipo == TipoAccion::EsquivarIzq ? VelocidadEsquivaExterior : VelocidadEsquivaInterior;
    }
    if (!activo || ahora - ultimo > 50) {
        inicio = ahora;
        errorAnterior = error;
    }
    activo = true;
    ultimo = ahora;
    const unsigned long transcurrido = ahora - inicio;
    const int8_t tramos = transcurrido >= 800 ? 8 : (unsigned int)transcurrido / 100;
    const int8_t x[redcontrol::NumEntradasRed] = {
        (int8_t)(error / 125),
        (int8_t)((error - errorAnterior) / 125),
        (int8_t)(empujando ? 8 : 0), tramos,
        (int8_t)(limiteBorde < 255 ? 8 : 0),
        (int8_t)(mando.velocidadAvanceEstimada() / 32),
        (int8_t)(d.tipo == TipoAccion::EsquivarIzq ? -8 : d.tipo == TipoAccion::EsquivarDer ? 8 : 0)
    };
    errorAnterior = error;
    int avance = 0, giro = 0;
    if (UsarRedControl && (modo != 1 || UsarRedControlModo1) &&
        (modo != 3 || UsarRedControlModo3))
        redcontrol::corregir(x, avance, giro);
    if (avance || giro) {
        const int base = (izq + der) / 2 + avance;
        const int vuelta = (der - izq) / 2 + giro;
        izq = limitar(base - vuelta, -255, 255);
        der = limitar(base + vuelta, -255, 255);
    }
    // Los límites de borde y la rampa siguen siendo la última etapa.
    mando.mover(motor, izq, der);
}
