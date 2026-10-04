#include "EstimadorBorde.H"
#include "Parametros.H"

#ifdef __wasm__
// En la página web del simulador no hay biblioteca estándar: las funciones las aporta JavaScript
#include "matematica.h"
using mat::atan2;
using mat::cos;
using mat::sin;
using mat::sqrt;
#else
#include <math.h>
#endif

#ifdef ODOMETRIA_SIMULADA
// Solo en el simulador: avance (m) y giro (rad) desde la última llamada como los
// medirían encoders en las ruedas y un giroscopio, para ver qué aportarían
bool leerOdometria(float& avance, float& giro);
#endif

namespace {
const float Pi = 3.14159265f;

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
    // Cada rueda se acerca a su velocidad de régimen con la constante de tiempo
    // del robot (motor, inercia y adherencia)
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
    // Integración con el rumbo a mitad del paso
    const float medio = th + w * dt / 2;
    px += v * cos(medio) * dt;
    py += v * sin(medio) * dt;
    th = envolver(th + w * dt);

    // Lo recorrido y lo girado se conocen solo aproximadamente (patinan las
    // ruedas, cambia la batería), y un enemigo encima mueve al robot sin que lo sepa
    sigPos += (absf(v) * ErrorDistancia + empujon) * dt;
    sigTh += (absf(w) * ErrorGiro + absf(v) * DerivaRumbo + empujon * 4) * dt;
    sigPos = minf(sigPos, RadioDohyo);
    sigTh = minf(sigTh, Pi);

    // Si sigue en pie, su centro está dentro del dohyo
    const float r = sqrt(px * px + py * py);
    if (r > RadioDohyo) {
        px *= RadioDohyo / r;
        py *= RadioDohyo / r;
    }
}

void EstimadorBorde::lineaVista(int lado, bool avanzando) {
    // Medida: rumbo respecto al radio hacia fuera (0 = de frente al borde,
    // positivo = girado a la izquierda). Si la toca primero el sensor derecho,
    // el robot mira hacia fuera girado a la izquierda, y al revés.
    float relMedido = lado > 0 ? AnguloLineaLado : lado < 0 ? -AnguloLineaLado : 0;
    float sigMedido = lado == 0 ? IncertAnguloLinea / 2 : IncertAnguloLinea;
    if (!avanzando) {
        // Girando o retrocediendo, los sensores delanteros pueden tocarla con
        // cualquier rumbo: la medida apenas informa del rumbo
        sigMedido = Pi / 2;
    }

    const float r = sqrt(px * px + py * py);
    const float phi = r > 0.001f ? atan2(py, px) : th - relMedido;
    const float relPredicho = envolver(th - phi);

    // Fusión de Kalman en una dimensión: pesa cada valor por la inversa de su varianza
    float rel = relMedido;
    float sigRel = sigMedido;
    if (sigTh < Pi * 0.6f) {
        const float vp = sigTh * sigTh;
        const float vm = sigMedido * sigMedido;
        const float ganancia = vp / (vp + vm);
        rel = envolver(relPredicho + ganancia * envolver(relMedido - relPredicho));
        sigRel = sqrt(vp * vm / (vp + vm));
    }

    // El sensor más adelantado hacia el borde está sobre el inicio de la línea
    const float rSensor = RadioDohyo - AnchoLineaBorde;
    float rNuevo = rSensor - (DistanciaSensorPiso * cos(rel) + SeparacionSensoresPiso * absf(sin(rel)));
    if (rNuevo < 0) rNuevo = 0;

    px = rNuevo * cos(phi);
    py = rNuevo * sin(phi);
    th = envolver(phi + rel);
    sigPos = IncertPosicionLinea;
    sigTh = sigRel;
}

float EstimadorBorde::distanciaLibre() const {
    const float limite = RadioDohyo - MargenBorde;
    const float r = sqrt(px * px + py * py);
    const float rPeor = minf(r + SigmasSeguridad * sigPos, RadioDohyo);
    if (rPeor >= limite) {
        return 0;
    }
    const float abanico = SigmasSeguridad * sigTh;
    if (abanico >= Pi || r < 0.001f) {
        // Rumbo desconocido: lo peor es ir derecho hacia fuera
        return limite - rPeor;
    }
    // Lo peor dentro del abanico de rumbos posibles es el más cercano a ir
    // derecho hacia fuera (la distancia al borde crece al apartarse de él)
    const float rel = envolver(th - atan2(py, px));
    float peor;
    if (absf(rel) <= abanico) {
        peor = 0;
    } else {
        peor = minf(absf(envolver(rel - abanico)), absf(envolver(rel + abanico)));
    }
    // Distancia desde un punto a radio rPeor hasta el círculo límite, saliendo
    // con un ángulo 'peor' respecto al radio
    const float s = rPeor * sin(peor);
    return -rPeor * cos(peor) + sqrt(limite * limite - s * s);
}

bool EstimadorBorde::confiable() const {
    return sigPos <= IncertPosicionConfiable && sigTh <= IncertRumboConfiable;
}

int EstimadorBorde::limiteAvance() const {
    // Recorre v·RetardoReaccion antes de empezar a frenar y luego v²/(2a)
    const float d = distanciaLibre();
    const float a = DesaceleracionFreno;
    const float aT = a * RetardoReaccion;
    const float v = -aT + sqrt(aT * aT + 2 * a * d);
    // Orden que daría esa velocidad aunque el robot sea más rápido que el modelo
    const float vMax = VelocidadRuedaMaxima * (1 + ErrorDistancia);
    long pwm = ZonaMuertaPwm + (long)(v / vMax * (255 - ZonaMuertaPwm));
    if (pwm < VelocidadMinimaEstimador) pwm = VelocidadMinimaEstimador;
    if (pwm > 255) pwm = 255;
    return (int)pwm;
}
