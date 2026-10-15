#include "movimiento/EstimadorBorde.h"
#include "Parametros.h"

// La tabla del seno va en Flash en el Leonardo (no ocupa RAM)
#if defined(__AVR__)
#include <avr/pgmspace.h>
#define LEER_TABLA(t, i) ((int)pgm_read_word(&(t)[i]))
#else
#define PROGMEM
#define LEER_TABLA(t, i) ((int)(t)[i])
#endif

// En el simulador se puede usar odometría "real" (encoders simulados)
#ifdef ODOMETRIA_SIMULADA
bool leerOdometria(float& avance, float& giro);
#endif

namespace {
constexpr long M = EstimadorBorde::Metro;
constexpr long Pi = EstimadorBorde::Vuelta / 2;
constexpr long CuartoVuelta = EstimadorBorde::Vuelta / 4;
constexpr long Uno = 16384;   // 1,0 de seno y coseno

constexpr long metros(float m) { return EstimadorBorde::metros(m); }
constexpr long radianes(float r) { return EstimadorBorde::radianes(r); }
constexpr long q10(float f) { return (long)(f * 1024 + 0.5f); }

// Parámetros de Parametros.h en unidades enteras
constexpr long Radio = metros(RadioDohyo);
constexpr long RadioSensor = metros(RadioDohyo - AnchoLineaBorde);
constexpr long DistSensor = metros(DistanciaSensorPiso);
constexpr long SepSensores = metros(SeparacionSensoresPiso);
constexpr long VelMaxima = metros(VelocidadRuedaMaxima);
constexpr long TauMs = (long)(TauRuedaEstimador * 1000 + 0.5f);
// Giro (Vuelta/s) por diferencia de velocidad entre ruedas (Metro/s), en 1/1024
constexpr long GiroPorDiferencia =
    (long)(EstimadorBorde::Vuelta / (6.28318531f * TrochaRuedas * M) * 1024 + 0.5f);
constexpr long ErrDistancia = q10(ErrorDistancia);
constexpr long ErrGiro = q10(ErrorGiro);
// Deriva del rumbo (rad/m) y efecto de los empujones (4 rad/m): Vuelta por Metro, en 1/65536
constexpr long Deriva = (long)(DerivaRumbo / 6.28318531f * 65536 + 0.5f);
constexpr long DerivaEmpujon = (long)(4 / 6.28318531f * 65536 + 0.5f);
constexpr long AnguloLado = radianes(AnguloLineaLado);
constexpr long IncertAngulo = radianes(IncertAnguloLinea);
constexpr long IncertLinea = metros(IncertPosicionLinea);
constexpr long Sigmas = (long)(SigmasSeguridad * 256 + 0.5f);
constexpr long Limite = metros(RadioDohyo - MargenBorde);
constexpr long PosConfiable = metros(IncertPosicionConfiable);
constexpr long RumboConfiable = radianes(IncertRumboConfiable);
constexpr long RMinimo = metros(0.001f);
// Frenada en 1/1024 m: aceleración, velocidad perdida en el retardo y velocidad máxima
constexpr long Frenada = (long)(DesaceleracionFreno * 1024 + 0.5f);
constexpr long FrenadaRetardo = (long)(DesaceleracionFreno * 1024 * RetardoReaccion + 0.5f);
constexpr long VelMaximaPeor = (long)(VelocidadRuedaMaxima * (1 + ErrorDistancia) * 1024 + 0.5f);

// Seno de 0 a 90° en 64 pasos (×16384)
const int16_t TablaSeno[65] PROGMEM = {
    0, 402, 804, 1205, 1606, 2006, 2404, 2801, 3196, 3590, 3981, 4370, 4756, 5139, 5520, 5897,
    6270, 6639, 7005, 7366, 7723, 8076, 8423, 8765, 9102, 9434, 9760, 10080, 10394, 10702, 11003, 11297,
    11585, 11866, 12140, 12406, 12665, 12916, 13160, 13395, 13623, 13842, 14053, 14256, 14449, 14635, 14811, 14978,
    15137, 15286, 15426, 15557, 15679, 15791, 15893, 15986, 16069, 16143, 16207, 16261, 16305, 16340, 16364, 16379,
    16384,
};

// Seno (×16384) de un ángulo en Vuelta, interpolando en la tabla
long seno(long angulo) {
    const uint16_t u = (uint16_t)angulo;
    const uint8_t cuadrante = u >> 14;
    uint16_t dentro = u & 0x3FFF;
    if (cuadrante & 1) dentro = 0x4000 - dentro;
    const uint8_t i = dentro >> 8;
    long v = LEER_TABLA(TablaSeno, i);
    if (i < 64) v += ((LEER_TABLA(TablaSeno, i + 1) - v) * (long)(dentro & 0xFF)) >> 8;
    return cuadrante & 2 ? -v : v;
}

long coseno(long angulo) { return seno(angulo + CuartoVuelta); }

// Raíz cuadrada entera
long raiz(unsigned long n) {
    unsigned long r = 0, bit = 1UL << 30;
    while (bit > n) bit >>= 2;
    while (bit) {
        if (n >= r + bit) {
            n -= r + bit;
            r = (r >> 1) + bit;
        } else {
            r >>= 1;
        }
        bit >>= 2;
    }
    return (long)r;
}

// atan2 en Vuelta (error < 0,25°): atan(z) ≈ z·(π/4 + 0,273·(1 − z)) para 0 ≤ z ≤ 1
long arcoTangente(long y, long x) {
    long ax = x < 0 ? -x : x, ay = y < 0 ? -y : y;
    if (ax == 0 && ay == 0) return 0;
    while (ax > 32767 || ay > 32767) {
        ax >>= 1;
        ay >>= 1;
    }
    const bool empinado = ay > ax;
    const long z = empinado ? (ax << 15) / ay : (ay << 15) / ax;   // 0..32768
    long a = (z * (8192 + ((2848L * (32768 - z)) >> 15))) >> 15;
    if (empinado) a = CuartoVuelta - a;
    if (x < 0) a = Pi - a;
    return y < 0 ? -a : a;
}

long absl(long v) { return v < 0 ? -v : v; }
long minl(long a, long b) { return a < b ? a : b; }
// a · num / den redondeado
long escalar(long a, long num, long den) {
    const long p = a * num;
    return (p >= 0 ? p + den / 2 : p - den / 2) / den;
}
}

EstimadorBorde::EstimadorBorde()
    : px(0), py(0), th(0), vIzq(0), vDer(0), sigPos(metros(RadioSalidaEstimado)), sigTh(Pi) {}

void EstimadorBorde::colocarQ(long radio, long sigmaPos, bool mirandoAlCentro, long sigmaRumbo) {
    // Sistema de coordenadas propio: el robot sobre el eje x
    px = radio;
    py = 0;
    th = (int16_t)(mirandoAlCentro ? Pi : 0);
    sigPos = sigmaPos;
    sigTh = mirandoAlCentro ? sigmaRumbo : Pi;
}

void EstimadorBorde::colocarGiradoQ(long radio, long sigmaPos, long desvio, long sigmaRumbo) {
    px = radio;
    py = 0;
    th = (int16_t)(Pi + desvio);
    sigPos = sigmaPos;
    sigTh = sigmaRumbo;
}

long EstimadorBorde::velocidadRegimenQ(int pwm) {
    const int magnitud = pwm < 0 ? -pwm : pwm;
    if (magnitud <= ZonaMuertaPwm) return 0;
    const long v = VelMaxima * (magnitud - ZonaMuertaPwm) / (255 - ZonaMuertaPwm);
    return pwm < 0 ? -v : v;
}

void EstimadorBorde::predecirMs(int pwmIzq, int pwmDer, unsigned int dtMs, long empujon) {
    const long dt = dtMs;
    // 1. Velocidad de las ruedas: se acercan a su velocidad de régimen con inercia
    const long vrIzq = velocidadRegimenQ(pwmIzq), vrDer = velocidadRegimenQ(pwmDer);
    if (dt >= TauMs) {
        vIzq = vrIzq;
        vDer = vrDer;
    } else {
        vIzq += escalar(vrIzq - vIzq, dt, TauMs);
        vDer += escalar(vrDer - vDer, dt, TauMs);
    }
    long v = (vIzq + vDer) / 2;
    long w = ((vDer - vIzq) * GiroPorDiferencia) >> 10;   // Vuelta/s
#ifdef ODOMETRIA_SIMULADA
    float avance, giro;
    if (dt > 0 && leerOdometria(avance, giro)) {
        v = metros(avance) * 1000 / dt;
        w = radianes(giro) * 1000 / dt;
    }
#endif
    // 2. Integrar posición y rumbo (con el rumbo a mitad del paso)
    const long medio = th + escalar(w, dt, 2000);
    px += escalar((v * coseno(medio)) >> 14, dt, 1000);
    py += escalar((v * seno(medio)) >> 14, dt, 1000);
    th = (int16_t)(th + escalar(w, dt, 1000));

    // 3. La incertidumbre crece con lo recorrido, lo girado y los empujones
    const long av = absl(v);
    sigPos += escalar(((av * ErrDistancia) >> 10) + empujon, dt, 1000);
    sigTh += escalar(((absl(w) * ErrGiro) >> 10) + ((av * Deriva) >> 16) + ((empujon * DerivaEmpujon) >> 16),
                     dt, 1000);
    sigPos = minl(sigPos, Radio);
    sigTh = minl(sigTh, Pi);

    // 4. El robot no puede estar fuera del dohyo
    const long r = raiz((unsigned long)(px * px) + (unsigned long)(py * py));
    if (r > Radio) {
        px = px * Radio / r;
        py = py * Radio / r;
    }
}

void EstimadorBorde::lineaVista(int lado, bool avanzando) {
    // 1. Ángulo con el que se cruza la línea según qué sensor la vio
    //    (retrocediendo no se sabe: incertidumbre máxima)
    const long relMedido = lado > 0 ? AnguloLado : lado < 0 ? -AnguloLado : 0;
    long sigMedido = lado == 0 ? IncertAngulo / 2 : IncertAngulo;
    if (!avanzando) {
        sigMedido = CuartoVuelta;
    }

    // 2. Combinar la medida con la predicción (como un filtro de Kalman)
    const long r = raiz((unsigned long)(px * px) + (unsigned long)(py * py));
    const long phi = r > RMinimo ? arcoTangente(py, px) : th - relMedido;
    const int16_t relPredicho = (int16_t)(th - phi);

    long rel = relMedido;
    long sigRel = sigMedido;
    if (sigTh < Pi * 6 / 10) {
        // Varianzas en (Vuelta/64)² para no desbordar; ganancia en 1/1024
        const long vp = (sigTh >> 6) * (sigTh >> 6);
        const long vm = (sigMedido >> 6) * (sigMedido >> 6);
        const long ganancia = vp + vm > 0 ? vp * 1024 / (vp + vm) : 512;
        rel = (int16_t)(relPredicho + ((ganancia * (int16_t)(relMedido - relPredicho)) >> 10));
        sigRel = raiz((unsigned long)((vp * (1024 - ganancia)) >> 10)) << 6;
    }

    // 3. El sensor está sobre la línea: colocar el robot en el radio correspondiente
    long rNuevo = RadioSensor - ((DistSensor * coseno(rel) + SepSensores * absl(seno(rel))) >> 14);
    if (rNuevo < 0) rNuevo = 0;

    px = (rNuevo * coseno(phi)) >> 14;
    py = (rNuevo * seno(phi)) >> 14;
    th = (int16_t)(phi + rel);
    sigPos = IncertLinea;
    sigTh = sigRel;
}

// Distancia hasta el borde en la peor posición y rumbo posibles
long EstimadorBorde::distanciaLibreQ() const {
    const long r = raiz((unsigned long)(px * px) + (unsigned long)(py * py));
    const long rPeor = minl(r + ((Sigmas * sigPos) >> 8), Radio);
    const long abanico = (Sigmas * sigTh) >> 8;
    if (abanico >= Pi || r < RMinimo) {
        return rPeor >= Limite ? 0 : Limite - rPeor;
    }
    const int16_t rel = (int16_t)(th - arcoTangente(py, px));
    long peor;
    if (absl(rel) <= abanico) {
        peor = 0;
    } else {
        peor = minl(absl((int16_t)(rel - abanico)), absl((int16_t)(rel + abanico)));
    }
    const long s = (rPeor * seno(peor)) >> 14;
    const long c = coseno(peor);
    // Ya en la zona de margen: solo hay sitio si mira hacia dentro y su
    // trayectoria vuelve a cruzar el círculo seguro (hasta el otro lado)
    if (rPeor >= Limite && (c >= 0 || absl(s) >= Limite)) {
        return 0;
    }
    return -((rPeor * c) >> 14) + raiz((unsigned long)(Limite * Limite - s * s));
}

bool EstimadorBorde::confiable() const {
    return sigPos <= PosConfiable && sigTh <= RumboConfiable;
}

// Velocidad con la que, frenando a DesaceleracionFreno tras RetardoReaccion,
// se para antes de recorrer distanciaLibre(); convertida a PWM. En 1/1024 m.
int EstimadorBorde::limiteAvance() const {
    const long d = distanciaLibreQ() >> 6;
    const long v = -FrenadaRetardo + raiz((unsigned long)(FrenadaRetardo * FrenadaRetardo + 2 * Frenada * d));
    long pwm = ZonaMuertaPwm + v * (255 - ZonaMuertaPwm) / VelMaximaPeor;
    if (pwm < VelocidadMinimaEstimador) pwm = VelocidadMinimaEstimador;
    if (pwm > 255) pwm = 255;
    return (int)pwm;
}
