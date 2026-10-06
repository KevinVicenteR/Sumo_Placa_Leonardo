#include "movimiento/EstimadorBorde.h"
#include "Parametros.h"

// En WebAssembly (simulador web) no hay math.h: se usan versiones propias
#ifdef __wasm__
#include "matematica.h"
using mat::atan2;
using mat::cos;
using mat::sin;
using mat::sqrt;
#else
#include <math.h>
#endif

// En el simulador se puede usar odometría "real" (encoders simulados)
#ifdef ODOMETRIA_SIMULADA
bool leerOdometria(float& avance, float& giro);
#endif

namespace {
const float Pi = 3.14159265f;

// Ángulo equivalente entre -π y π
float envolver(float a) {
    while (a > Pi) a -= 2 * Pi;
    while (a < -Pi) a += 2 * Pi;
    return a;
}

float absf(float v) { return v < 0 ? -v : v; }
float minf(float a, float b) { return a < b ? a : b; }
}

EstimadorBorde::EstimadorBorde()
    : px(0), py(0), th(0), vIzq(0), vDer(0), sigPos(RadioSalidaEstimado), sigTh(Pi) {}

float EstimadorBorde::velocidadRegimen(int pwm) {
    const int magnitud = pwm < 0 ? -pwm : pwm;
    if (magnitud <= ZonaMuertaPwm) return 0;
    const float v = VelocidadRuedaMaxima * (magnitud - ZonaMuertaPwm) / (255 - ZonaMuertaPwm);
    return pwm < 0 ? -v : v;
}

void EstimadorBorde::predecir(int pwmIzq, int pwmDer, float dt, float empujon) {
    // 1. Velocidad de las ruedas: se acercan a su velocidad de régimen con inercia
    const float paso = dt >= TauRuedaEstimador ? 1 : dt / TauRuedaEstimador;
    vIzq += (velocidadRegimen(pwmIzq) - vIzq) * paso;
    vDer += (velocidadRegimen(pwmDer) - vDer) * paso;
    float v = (vIzq + vDer) / 2;
    float w = (vDer - vIzq) / TrochaRuedas;
#ifdef ODOMETRIA_SIMULADA
    float avance, giro;
    if (dt > 0 && leerOdometria(avance, giro)) {
        v = avance / dt;
        w = giro / dt;
    }
#endif
    // 2. Integrar posición y rumbo (con el rumbo a mitad del paso)
    const float medio = th + w * dt / 2;
    px += v * cos(medio) * dt;
    py += v * sin(medio) * dt;
    th = envolver(th + w * dt);

    // 3. La incertidumbre crece con lo recorrido, lo girado y los empujones
    sigPos += (absf(v) * ErrorDistancia + empujon) * dt;
    sigTh += (absf(w) * ErrorGiro + absf(v) * DerivaRumbo + empujon * 4) * dt;
    sigPos = minf(sigPos, RadioDohyo);
    sigTh = minf(sigTh, Pi);

    // 4. El robot no puede estar fuera del dohyo
    const float r = sqrt(px * px + py * py);
    if (r > RadioDohyo) {
        px *= RadioDohyo / r;
        py *= RadioDohyo / r;
    }
}

void EstimadorBorde::lineaVista(int lado, bool avanzando) {
    // 1. Ángulo con el que se cruza la línea según qué sensor la vio
    //    (retrocediendo no se sabe: incertidumbre máxima)
    float relMedido = lado > 0 ? AnguloLineaLado : lado < 0 ? -AnguloLineaLado : 0;
    float sigMedido = lado == 0 ? IncertAnguloLinea / 2 : IncertAnguloLinea;
    if (!avanzando) {
        sigMedido = Pi / 2;
    }

    // 2. Combinar la medida con la predicción (como un filtro de Kalman)
    const float r = sqrt(px * px + py * py);
    const float phi = r > 0.001f ? atan2(py, px) : th - relMedido;
    const float relPredicho = envolver(th - phi);

    float rel = relMedido;
    float sigRel = sigMedido;
    if (sigTh < Pi * 0.6f) {
        const float vp = sigTh * sigTh;
        const float vm = sigMedido * sigMedido;
        const float ganancia = vp / (vp + vm);
        rel = envolver(relPredicho + ganancia * envolver(relMedido - relPredicho));
        sigRel = sqrt(vp * vm / (vp + vm));
    }

    // 3. El sensor está sobre la línea: colocar el robot en el radio correspondiente
    const float rSensor = RadioDohyo - AnchoLineaBorde;
    float rNuevo = rSensor - (DistanciaSensorPiso * cos(rel) + SeparacionSensoresPiso * absf(sin(rel)));
    if (rNuevo < 0) rNuevo = 0;

    px = rNuevo * cos(phi);
    py = rNuevo * sin(phi);
    th = envolver(phi + rel);
    sigPos = IncertPosicionLinea;
    sigTh = sigRel;
}

// Distancia hasta el borde en la peor posición y rumbo posibles
float EstimadorBorde::distanciaLibre() const {
    const float limite = RadioDohyo - MargenBorde;
    const float r = sqrt(px * px + py * py);
    const float rPeor = minf(r + SigmasSeguridad * sigPos, RadioDohyo);
    if (rPeor >= limite) {
        return 0;
    }
    const float abanico = SigmasSeguridad * sigTh;
    if (abanico >= Pi || r < 0.001f) {
        return limite - rPeor;
    }
    const float rel = envolver(th - atan2(py, px));
    float peor;
    if (absf(rel) <= abanico) {
        peor = 0;
    } else {
        peor = minf(absf(envolver(rel - abanico)), absf(envolver(rel + abanico)));
    }
    const float s = rPeor * sin(peor);
    return -rPeor * cos(peor) + sqrt(limite * limite - s * s);
}

bool EstimadorBorde::confiable() const {
    return sigPos <= IncertPosicionConfiable && sigTh <= IncertRumboConfiable;
}

// Velocidad con la que, frenando a DesaceleracionFreno tras RetardoReaccion,
// se para antes de recorrer distanciaLibre(); convertida a PWM
int EstimadorBorde::limiteAvance() const {
    const float d = distanciaLibre();
    const float a = DesaceleracionFreno;
    const float aT = a * RetardoReaccion;
    const float v = -aT + sqrt(aT * aT + 2 * a * d);
    const float vMax = VelocidadRuedaMaxima * (1 + ErrorDistancia);
    long pwm = ZonaMuertaPwm + (long)(v / vMax * (255 - ZonaMuertaPwm));
    if (pwm < VelocidadMinimaEstimador) pwm = VelocidadMinimaEstimador;
    if (pwm > 255) pwm = 255;
    return (int)pwm;
}
