// Física del robot y del dohyo, y Arduino simulado.
//
// La comparten el simulador de consola (sim.cpp) y el de la página web
// (web/sim_web.cpp), para que una misma semilla dé el mismo combate en ambos.
// Cada programa debe incluir este archivo una sola vez: define las funciones
// de Arduino (millis, analogRead, digitalWrite...) que usa el firmware.

#ifndef SIMULACION_FISICA_H
#define SIMULACION_FISICA_H

#include <Arduino.h>
#include "matematica.h"
#include "Pines.h"
#include "Parametros.h"

void setup();
void loop();

namespace sim {

constexpr double PI = 3.14159265358979323846;

struct Config {
    // Dohyo
    double radio = 0.385;         // m (77 cm de diámetro, borde incluido: minisumo reglamentario)
    double borde = 0.025;         // m de línea blanca (2,5 cm reglamentarios)
    // Robot
    double largo = 0.10;          // m
    double ancho = 0.10;          // m
    double trocha = 0.085;        // m entre ruedas
    double sensorPisoX = 0.045;   // m hacia adelante desde el eje de ruedas
    double sensorPisoY = 0.040;   // m a cada lado
    double vmax = 0.8;            // m/s con PWM 255
    double tau = 0.05;            // s, constante de tiempo del motor
    double mu = 0.9;              // adherencia rueda-piso (limita aceleración)
    // Modelo de motor DC (se activa con rpm > 0; si no, se usa vmax/tau)
    double rpm = 0;               // rpm en vacío del motorreductor
    double diametro = 0.03;       // m, diámetro de rueda
    double masa = 0.3;            // kg
    double parBloqueo = 0.6;      // kg·cm por motor con el rotor bloqueado
    double bateria = 1.0;         // tensión de batería / tensión nominal del motor
    double friccionCaja = 0.15;   // fricción de la reductora (fracción de la fuerza de bloqueo)
    double rodadura = 0.03;       // coeficiente de resistencia a la rodadura
    double friccionGiro = 0.5;    // roce lateral al girar en el sitio (ruedas, pala)
    double brazoGiro = 0.012;     // m, brazo efectivo de ese roce
    // Sensores de piso (lecturas ADC) y su mancha de lectura
    double adcNegro = 900;
    double adcBlanco = 100;
    double adcFuera = 1000;       // fuera del dohyo, sin superficie debajo
    double manchaSensor = 0.003;  // m de radio del área que ve el sensor de piso
    // Objetos fuera del dohyo que ven los sensores de enemigo (0 = ninguno)
    double radioPared = 0;        // m desde el centro
    // Posición inicial: radio máximo del centro del robot; el cuerpo entero queda dentro del dohyo
    double radioInicio = 0.12;
    double rangoEnemigo = 0.40;   // m, alcance de los sensores de enemigo
    bool enemigoInvertido = false; // sensores de enemigo que dan LOW al detectar
    // Driver de motores: false = TB6612 (PWM a 0 frena), true = L298N (PWM a 0,
    // es decir enable a 0, deja el motor libre, y mientras conduce con PWM no frena)
    bool driverL298 = false;
    // Posición de salida: 0 aleatoria, 1 espalda con espalda, 2 lado a lado
    // (mirando en sentidos opuestos), 3 enfrentados cerca de los bordes
    int salida = 0;
    int ladoRival = 1;            // round 2: +1 rival a la derecha, -1 a la izquierda
    int dip = 0;                  // interruptores DIP en ON (bit 0 = DIP1...)
    // Pines de los DIP (XMotion: 5, 6, 7) y si ON lee LOW; deben coincidir con
    // Pines.h y DipActivoBajo del firmware (aquí fijos para poder simular
    // también firmwares anteriores que no los tienen)
    uint8_t pinesDip[3] = {5, 6, 7};
    bool dipActivoBajo = true;
    // Detecciones fantasma (reflejos, manos, parpadeos): cada sensor de enemigo
    // ve algo que no existe, en promedio fantasmasPorSegundo veces por segundo,
    // durante entre 5 y 30 ms
    double fantasmasPorSegundo = 0;
    // Saturación por el infrarrojo del rival: cada sensor de enemigo se queda
    // fijo en "detecta", en promedio saturacionesPorSegundo veces por segundo,
    // durante entre 0,5 y 2 s
    double saturacionesPorSegundo = 0;
    // Cortes en la señal del módulo de arranque (interferencias): la señal pasa a
    // STOP cortesArranquePorSegundo veces por segundo, durante entre 5 ms y
    // corteArranqueMaxMs
    double cortesArranquePorSegundo = 0;
    double corteArranqueMaxMs = 300;
    // Desgaste de cada llanta (0 = nueva, 1 = gastada del todo): pierde agarre
    // en esa proporción y hasta un 10 % de diámetro
    double desgasteIzq = 0;
    double desgasteDer = 0;
    // Enemigo
    double radioEnemigo = 0.05;   // m
    double velEnemigo = 0.25;     // m/s en modo errante
    double velAgresivo = 0.6;     // m/s del enemigo que embiste (modo agresivo)
    double giroAgresivo = 6.0;    // rad/s máximos con los que se orienta hacia nosotros
    // Choque con el enemigo (0 en masaEnemigo = fantasma, el robot lo atraviesa)
    double masaEnemigo = 0.3;     // kg
    double agarreEnemigo = 0.8;   // su resistencia a ser empujado (μ de sus ruedas)
    // Pala delantera de nuestro robot (base biselada que roza el piso): si el
    // rival choca por delante (dentro de ±35°), la pala se mete debajo, lo
    // levanta un poco y pierde esta fracción de su agarre (0 = sin pala)
    double pala = 0;
    // Pala del rival: si nos choca por su frente, perdemos esta fracción de agarre
    double palaRival = 0;
    double radioChoque = 0.055;   // m, radio de choque de nuestro robot
    double rigidezChoque = 3000;  // N/m del contacto entre robots (unos mm de solape)
    double amortiguaChoque = 25;  // N·s/m
    // Simulación
    double duracion = 30.0;       // s de combate (tras los 5 s reglamentarios)
    double ruidoPiso = 15.0;      // desviación estándar del ADC
    double retardoPiso = 0;       // s que tarda el sensor de piso en reflejar lo que tiene debajo
    double sobrecostoLoopUs = 20; // µs de loop() además de las lecturas
};

// Flanqueo: como Agresivo, pero primero rodea a nuestro robot para golpearlo de lado
enum class Modo { Ninguno, Estatico, Errante, Agresivo, Flanqueo };

struct Robot {
    double x, y, th;  // posición del eje de ruedas y orientación (rad, antihorario)
    double vl, vr;    // velocidad real de cada rueda
    double vlat = 0;  // deslizamiento lateral (hacia la izquierda), solo si lo empujan de lado
    double fExt = 0;  // fuerza del enemigo a lo largo del rumbo en este paso (N)
};

struct Enemigo {
    bool presente;
    double x, y, th;
    double vx = 0, vy = 0;  // velocidad (la cambian sus motores y nuestros empujones)
};

// xorshift64* con Box-Muller: igual en todas las plataformas
class Aleatorio {
public:
    void sembrar(unsigned long long semilla) {
        estado = semilla * 0x9E3779B97F4A7C15ULL + 1;
        hayNormal = false;
    }
    double uniforme() {
        estado ^= estado >> 12;
        estado ^= estado << 25;
        estado ^= estado >> 27;
        return ((estado * 0x2545F4914F6CDD1DULL) >> 11) * (1.0 / 9007199254740992.0);
    }
    double entre(double a, double b) { return a + (b - a) * uniforme(); }
    double normal() {
        if (hayNormal) {
            hayNormal = false;
            return normalGuardada;
        }
        double u1 = uniforme();
        if (u1 < 1e-300) u1 = 1e-300;
        const double u2 = uniforme();
        const double r = mat::sqrt(-2.0 * mat::log(u1));
        normalGuardada = r * mat::sin(2 * PI * u2);
        hayNormal = true;
        return r * mat::cos(2 * PI * u2);
    }

private:
    unsigned long long estado = 1;
    bool hayNormal = false;
    double normalGuardada = 0;
};

inline double minimo(double a, double b) { return a < b ? a : b; }
inline double maximo(double a, double b) { return a > b ? a : b; }

inline Config cfg;
inline Modo modo = Modo::Ninguno;
inline Robot rob;
inline Enemigo ene;

// Nuestra pala está debajo del rival en este paso (ver Config::pala)
inline bool palaDebajo = false;
// La pala del rival está debajo de nuestro robot (ver Config::palaRival)
inline bool palaRivalDebajo = false;
inline double agarrePropio() { return palaRivalDebajo ? 1 - cfg.palaRival : 1; }

inline Aleatorio azar;

inline double tiempoUs = 0;
inline double costoLoopUs = 0;
inline uint8_t nivelPin[32];
inline int pwmPin[32];

// Posiciones recientes del robot, para simular sensores de piso lentos
struct Pose {
    double t, x, y, th;
};
constexpr int MaxHistorial = 1024;
inline Pose historial[MaxHistorial];
inline int historialIni = 0, historialN = 0;

inline void guardarPose() {
    const int i = (historialIni + historialN) % MaxHistorial;
    historial[i] = {tiempoUs / 1e6, rob.x, rob.y, rob.th};
    if (historialN < MaxHistorial) historialN++;
    else historialIni = (historialIni + 1) % MaxHistorial;
}

// Pose del robot hace cfg.retardoPiso segundos (la más reciente si no hay retardo)
inline Pose poseRetrasada() {
    Pose p = {tiempoUs / 1e6, rob.x, rob.y, rob.th};
    if (cfg.retardoPiso <= 0) return p;
    const double objetivo = tiempoUs / 1e6 - cfg.retardoPiso;
    for (int k = historialN - 1; k >= 0; k--) {
        const Pose& h = historial[(historialIni + k) % MaxHistorial];
        p = h;
        if (h.t <= objetivo) break;
    }
    return p;
}

inline void aMundo(double rx, double ry, double& wx, double& wy) {
    const double c = mat::cos(rob.th), s = mat::sin(rob.th);
    wx = rob.x + c * rx - s * ry;
    wy = rob.y + s * rx + c * ry;
}

// ---------------------------------------------------------------- sensores

inline double valorPiso(double wx, double wy) {
    const double r = mat::hypot(wx, wy);
    if (r <= cfg.radio - cfg.borde) return cfg.adcNegro;
    if (r <= cfg.radio) return cfg.adcBlanco;
    return cfg.adcFuera;
}

// El sensor promedia un área pequeña, así que la transición negro-blanco no es instantánea
inline int lecturaPiso(double rx, double ry) {
    const Pose p = poseRetrasada();
    const double c = mat::cos(p.th), sn = mat::sin(p.th);
    const double wx = p.x + c * rx - sn * ry;
    const double wy = p.y + sn * rx + c * ry;
    const double m = cfg.manchaSensor;
    double valor = (valorPiso(wx, wy) * 2 + valorPiso(wx + m, wy) + valorPiso(wx - m, wy) +
                    valorPiso(wx, wy + m) + valorPiso(wx, wy - m)) / 6;
    valor += azar.normal() * cfg.ruidoPiso;
    return (int)constrain(valor, 0.0, 1023.0);
}

inline bool sobreBlanco(double rx, double ry) {
    double wx, wy;
    aMundo(rx, ry, wx, wy);
    const double r = mat::hypot(wx, wy);
    return r > cfg.radio - cfg.borde && r <= cfg.radio;
}

inline bool sensorPisoSobreBlanco() {
    return sobreBlanco(cfg.sensorPisoX, cfg.sensorPisoY) || sobreBlanco(cfg.sensorPisoX, -cfg.sensorPisoY);
}

// Rayo desde (rx, ry) en el marco del robot con ángulo relativo ang
inline bool rayoVeEnemigo(double rx, double ry, double ang) {
    if (!ene.presente) return false;
    double ox, oy;
    aMundo(rx, ry, ox, oy);
    const double ux = mat::cos(rob.th + ang), uy = mat::sin(rob.th + ang);
    const double dx = ene.x - ox, dy = ene.y - oy;
    const double t = dx * ux + dy * uy;
    const double perp2 = dx * dx + dy * dy - t * t;
    const double r2 = cfg.radioEnemigo * cfg.radioEnemigo;
    if (perp2 > r2) return false;
    const double entrada = t - mat::sqrt(r2 - perp2);
    const double salida = t + mat::sqrt(r2 - perp2);
    return salida >= 0 && entrada <= cfg.rangoEnemigo;
}

// Objetos alrededor del dohyo (pared, muebles, personas) como un círculo de radio radioPared
inline bool rayoVePared(double rx, double ry, double ang) {
    if (cfg.radioPared <= 0) return false;
    double ox, oy;
    aMundo(rx, ry, ox, oy);
    const double ux = mat::cos(rob.th + ang), uy = mat::sin(rob.th + ang);
    // Distancia hasta salir del círculo: |o + t·u| = R
    const double b = ox * ux + oy * uy;
    const double c = ox * ox + oy * oy - cfg.radioPared * cfg.radioPared;
    const double t = -b + mat::sqrt(b * b - c);
    return t <= cfg.rangoEnemigo;
}

inline bool rayoVe(double rx, double ry, double ang) {
    return rayoVeEnemigo(rx, ry, ang) || rayoVePared(rx, ry, ang);
}

inline double finFantasma[5] = {0, 0, 0, 0, 0};
inline double ultimaRevisionFantasma[5] = {0, 0, 0, 0, 0};

inline bool fantasma(int sensor) {
    if (cfg.fantasmasPorSegundo <= 0) return false;
    const double t = tiempoUs / 1e6;
    const double dt = t - ultimaRevisionFantasma[sensor];
    ultimaRevisionFantasma[sensor] = t;
    if (t >= finFantasma[sensor] && azar.uniforme() < cfg.fantasmasPorSegundo * dt) {
        finFantasma[sensor] = t + azar.entre(0.005, 0.030);
    }
    return t < finFantasma[sensor];
}

inline double finCorteArranque = 0;
inline double ultimaRevisionCorte = 0;

// La señal del módulo de arranque está cortada (lee STOP aunque el árbitro dio START)
inline bool corteArranque() {
    if (cfg.cortesArranquePorSegundo <= 0) return false;
    const double t = tiempoUs / 1e6;
    const double dt = t - ultimaRevisionCorte;
    ultimaRevisionCorte = t;
    if (t >= finCorteArranque && azar.uniforme() < cfg.cortesArranquePorSegundo * dt) {
        finCorteArranque = t + azar.entre(0.005, cfg.corteArranqueMaxMs / 1000);
    }
    return t < finCorteArranque;
}

inline double finSaturacion[5] = {0, 0, 0, 0, 0};
inline double ultimaRevisionSaturacion[5] = {0, 0, 0, 0, 0};

inline bool saturado(int sensor) {
    if (cfg.saturacionesPorSegundo <= 0) return false;
    const double t = tiempoUs / 1e6;
    const double dt = t - ultimaRevisionSaturacion[sensor];
    ultimaRevisionSaturacion[sensor] = t;
    if (t >= finSaturacion[sensor] && azar.uniforme() < cfg.saturacionesPorSegundo * dt) {
        finSaturacion[sensor] = t + azar.entre(0.5, 2.0);
    }
    return t < finSaturacion[sensor];
}

inline bool sensorEnemigo(uint8_t pin) {
    const double fx = cfg.largo / 2, ly = cfg.ancho / 2;
    const uint8_t pines[5] = {S_FRONT_CEN, S_FRONT_IZQ, S_FRONT_DER, S_LAT_IZQ, S_LAT_DER};
    for (int i = 0; i < 5; i++) {
        if (pin == pines[i] && (fantasma(i) || saturado(i))) return true;
    }
    if (pin == S_FRONT_CEN) return rayoVe(fx, 0, 0);
    if (pin == S_FRONT_IZQ) return rayoVe(fx, ly * 0.6, PI / 4);
    if (pin == S_FRONT_DER) return rayoVe(fx, -ly * 0.6, -PI / 4);
    if (pin == S_LAT_IZQ) return rayoVe(0, ly, PI / 2);
    if (pin == S_LAT_DER) return rayoVe(0, -ly, -PI / 2);
    return false;
}

// ---------------------------------------------------------------- física

inline int comandoMotor(uint8_t in1, uint8_t in2, uint8_t pwm) {
    if (nivelPin[in1] == nivelPin[in2]) return 0;
    return nivelPin[in1] == HIGH ? pwmPin[pwm] : -pwmPin[pwm];
}

enum class Puente { Conduce, Frena, Libre };
inline Puente estadoPuente(uint8_t pwm) {
    // Modelo conservador: PWM=0 sin fuerza eléctrica; validar desaceleración real.
    return pwmPin[pwm] == 0 ? Puente::Libre : Puente::Conduce;
}

// Motores cableados al revés (InvertirMotor* del firmware): la rueda gira al
// contrario de lo que indican los pines, y el firmware lo compensa
inline int comandoIzq() { return (InvertirMotorIzq ? -1 : 1) * comandoMotor(MA1A, MA2A, PWMA); }
inline int comandoDer() { return (InvertirMotorDer ? -1 : 1) * comandoMotor(MA1B, MA2B, PWMB); }

// Aplica una fricción de Coulomb de magnitud f a un movimiento con velocidad vel
// y fuerza impulsora fuerza. Si está quieto y la fuerza no supera la fricción, no arranca.
inline double conFriccion(double fuerza, double vel, double f) {
    if (mat::fabs(vel) < 1e-4) {
        if (mat::fabs(fuerza) <= f) return 0;
        return fuerza - (fuerza > 0 ? f : -f);
    }
    return fuerza - (vel > 0 ? f : -f);
}

// Motor DC: F = Fb·(u·batería − v/v0), menos la fricción de la reductora,
// limitada por la adherencia de cada rueda. desgaste = el de esa llanta.
inline double fuerzaRueda(int comando, double vRueda, Puente puente = Puente::Conduce, double desgaste = 0) {
    const double u = (puente == Puente::Conduce ? comando : 0) / 255.0 * cfg.bateria;
    const double diametro = cfg.diametro * (1 - 0.1 * desgaste);
    const double v0 = cfg.rpm / 60.0 * PI * diametro;
    const double fuerzaBloqueo = cfg.parBloqueo * 0.0981 / (diametro / 2);
    double motor = fuerzaBloqueo * (u - vRueda / v0);
    // Libre: el motor no hace fuerza. L298N conduciendo con PWM: en la parte baja
    // del PWM el motor queda libre y no puede frenar, solo empujar
    if (puente == Puente::Libre) motor = 0;
    if (puente == Puente::Conduce && cfg.driverL298 && motor * u < 0) motor = 0;
    double f = conFriccion(motor, vRueda, cfg.friccionCaja * fuerzaBloqueo);
    const double adherencia = cfg.mu * (1 - desgaste) * agarrePropio() * cfg.masa / 2 * 9.81;
    return constrain(f, -adherencia, adherencia);
}

// Cuerpo rígido: avance v y giro w con rodadura y roce lateral al girar
inline void avanzarCuerpoDC(int cmdIzq, int cmdDer, double dt) {
    const double b = cfg.trocha / 2;
    double v = (rob.vl + rob.vr) / 2;
    double w = (rob.vr - rob.vl) / cfg.trocha;
    const double fi = fuerzaRueda(cmdIzq, rob.vl, estadoPuente(PWMA), cfg.desgasteIzq);
    const double fd = fuerzaRueda(cmdDer, rob.vr, estadoPuente(PWMB), cfg.desgasteDer);
    const double peso = cfg.masa * 9.81;
    const double inercia = cfg.masa * (cfg.largo * cfg.largo + cfg.ancho * cfg.ancho) / 12;
    const double fuerza = conFriccion(fi + fd + rob.fExt, v, cfg.rodadura * peso);
    const double par = conFriccion((fd - fi) * b, w, cfg.friccionGiro * peso * cfg.brazoGiro);
    double vNueva = v + fuerza / cfg.masa * dt;
    double wNueva = w + par / inercia * dt;
    // La fricción frena hasta cero, no invierte el movimiento
    if (v != 0 && vNueva * v < 0 && mat::fabs(fi + fd + rob.fExt) < cfg.rodadura * peso) vNueva = 0;
    if (w != 0 && wNueva * w < 0 && mat::fabs((fd - fi) * b) < cfg.friccionGiro * peso * cfg.brazoGiro) wNueva = 0;
    rob.vl = vNueva - wNueva * b;
    rob.vr = vNueva + wNueva * b;
}

inline void avanzarRueda(double& v, int comando, double dt, double desgaste = 0) {
    const double objetivo = comando / 255.0 * cfg.vmax * (1 - 0.1 * desgaste);
    double dv = (objetivo - v) * dt / cfg.tau;
    const double maxDv = cfg.mu * (1 - desgaste) * 9.81 * dt;
    dv = constrain(dv, -maxDv, maxDv);
    v += dv;
}

inline void avanzarRobot(double dt) {
    if (cfg.rpm > 0) {
        avanzarCuerpoDC(comandoIzq(), comandoDer(), dt);
    } else {
        avanzarRueda(rob.vl, comandoIzq(), dt, cfg.desgasteIzq);
        avanzarRueda(rob.vr, comandoDer(), dt, cfg.desgasteDer);
        rob.vl += rob.fExt / cfg.masa * dt;
        rob.vr += rob.fExt / cfg.masa * dt;
    }
    const double v = (rob.vl + rob.vr) / 2;
    const double w = (rob.vr - rob.vl) / cfg.trocha;
    const double c = mat::cos(rob.th), s = mat::sin(rob.th);
    rob.x += (v * c - rob.vlat * s) * dt;
    rob.y += (v * s + rob.vlat * c) * dt;
    rob.th += w * dt;
}

// El enemigo intenta ir a su velocidad deseada (quieto o deambulando) con la
// aceleración que le permiten sus ruedas; esa misma adherencia es la que se
// opone a que lo empujen.
inline void avanzarEnemigo(double dt) {
    if (!ene.presente) return;
    double vdx = 0, vdy = 0;
    if (modo == Modo::Errante) {
        ene.th += azar.normal() * 3.0 * mat::sqrt(dt);
        const double r = mat::hypot(ene.x, ene.y);
        if (r > cfg.radio - 0.08) {
            // Se aleja del borde girando hacia el centro
            const double haciaCentro = mat::atan2(-ene.y, -ene.x);
            double diff = mat::remainder(haciaCentro - ene.th, 2 * PI);
            ene.th += diff * minimo(1.0, 8.0 * dt);
        }
        vdx = cfg.velEnemigo * mat::cos(ene.th);
        vdy = cfg.velEnemigo * mat::sin(ene.th);
    } else if (modo == Modo::Agresivo || modo == Modo::Flanqueo) {
        // Se orienta hacia nosotros con giro limitado y embiste; cerca del borde,
        // si no nos está empujando, frena y gira hacia el centro como un robot real
        double objetivoX = rob.x, objetivoY = rob.y;
        if (modo == Modo::Flanqueo && mat::hypot(rob.x - ene.x, rob.y - ene.y) > 0.18) {
            // Lejos: apunta a 12 cm del costado de nuestro robot que tiene más cerca
            const double izqX = -mat::sin(rob.th), izqY = mat::cos(rob.th);
            const double lado = (ene.x - rob.x) * izqX + (ene.y - rob.y) * izqY >= 0 ? 1.0 : -1.0;
            objetivoX += lado * 0.12 * izqX;
            objetivoY += lado * 0.12 * izqY;
        }
        const double haciaRobot = mat::atan2(objetivoY - ene.y, objetivoX - ene.x);
        double diff = mat::remainder(haciaRobot - ene.th, 2 * PI);
        const double maxGiro = cfg.giroAgresivo * dt;
        ene.th += diff > maxGiro ? maxGiro : diff < -maxGiro ? -maxGiro : diff;
        double vel = cfg.velAgresivo;
        const double r = mat::hypot(ene.x, ene.y);
        const double dRobot = mat::hypot(rob.x - ene.x, rob.y - ene.y);
        const double radial = (ene.x * mat::cos(ene.th) + ene.y * mat::sin(ene.th)) / (r > 1e-9 ? r : 1);
        if (r > cfg.radio - 0.07 && radial > 0 && dRobot > cfg.radioChoque + cfg.radioEnemigo + 0.01) {
            vel = 0;
            const double haciaCentro = mat::atan2(-ene.y, -ene.x);
            ene.th += mat::remainder(haciaCentro - ene.th, 2 * PI) * minimo(1.0, 8.0 * dt);
        }
        vdx = vel * mat::cos(ene.th);
        vdy = vel * mat::sin(ene.th);
    }
    if (cfg.masaEnemigo <= 0) {
        ene.vx = vdx;
        ene.vy = vdy;
    } else {
        const double dvx = vdx - ene.vx, dvy = vdy - ene.vy;
        const double dv = mat::hypot(dvx, dvy);
        const double agarre = cfg.agarreEnemigo * (palaDebajo ? 1 - cfg.pala : 1);
        const double maxDv = agarre * 9.81 * dt;
        const double f = dv > maxDv ? maxDv / dv : 1.0;
        ene.vx += dvx * f;
        ene.vy += dvy * f;
    }
    ene.x += ene.vx * dt;
    ene.y += ene.vy * dt;
}

// Contacto entre los dos robots (como círculos): una fuerza elástica con
// amortiguación a lo largo de la línea que une sus centros. Sobre nuestro robot,
// la parte a lo largo del rumbo se suma a la de las ruedas (lo frena o lo
// arrastra) y la lateral solo lo desliza de lado si supera el agarre lateral
// de las ruedas. El enemigo la compara con su propio agarre en avanzarEnemigo.
inline void resolverChoque(double dt) {
    rob.fExt = 0;
    double fLat = 0;
    palaDebajo = false;
    palaRivalDebajo = false;
    if (ene.presente && cfg.masaEnemigo > 0) {
        const double dx = ene.x - rob.x, dy = ene.y - rob.y;
        const double d = mat::hypot(dx, dy);
        const double minima = cfg.radioChoque + cfg.radioEnemigo;
        if (d < minima && d > 1e-9) {
            const double nx = dx / d, ny = dy / d;
            const double c = mat::cos(rob.th), s = mat::sin(rob.th);
            const double v = (rob.vl + rob.vr) / 2;
            const double vrx = v * c - rob.vlat * s, vry = v * s + rob.vlat * c;
            const double acercamiento = (vrx - ene.vx) * nx + (vry - ene.vy) * ny;
            double f = cfg.rigidezChoque * (minima - d) + cfg.amortiguaChoque * acercamiento;
            if (f < 0) f = 0;  // solo empuja, no tira
            // El rival está por delante de nuestro robot: la pala va debajo
            palaDebajo = cfg.pala > 0 && f > 0 && nx * c + ny * s > 0.82;
            // Nosotros estamos por delante del rival: su pala va debajo
            palaRivalDebajo = cfg.palaRival > 0 && f > 0 &&
                              -(nx * mat::cos(ene.th) + ny * mat::sin(ene.th)) > 0.82;
            ene.vx += f * nx / cfg.masaEnemigo * dt;
            ene.vy += f * ny / cfg.masaEnemigo * dt;
            rob.fExt = -f * (nx * c + ny * s);
            fLat = -f * (-nx * s + ny * c);
        }
    }
    // Deslizamiento lateral con fricción de Coulomb de todas las ruedas
    const double agarre = cfg.mu * agarrePropio() * cfg.masa * 9.81;
    const double neta = conFriccion(fLat, rob.vlat, agarre);
    const double nueva = rob.vlat + neta / cfg.masa * dt;
    rob.vlat = (rob.vlat != 0 && nueva * rob.vlat < 0) ? 0 : nueva;
}

inline bool enemigoFuera() { return ene.presente && mat::hypot(ene.x, ene.y) > cfg.radio; }

inline double maxRadioCuerpo() {
    double m = 0;
    const double xs[2] = {-cfg.largo / 2, cfg.largo / 2};
    const double ys[2] = {-cfg.ancho / 2, cfg.ancho / 2};
    for (double rx : xs) {
        for (double ry : ys) {
            double wx, wy;
            aMundo(rx, ry, wx, wy);
            m = maximo(m, mat::hypot(wx, wy));
        }
    }
    return m;
}

// ---------------------------------------------------------------- corrida

// Estado inicial: pines a cero, tiempo cero, robot y enemigo quietos
inline void reiniciar(unsigned long long semilla) {
    azar.sembrar(semilla);
    for (int i = 0; i < 32; i++) {
        nivelPin[i] = 0;
        pwmPin[i] = 0;
    }
    tiempoUs = 0;
    historialIni = historialN = 0;
    for (int i = 0; i < 5; i++) finFantasma[i] = ultimaRevisionFantasma[i] = 0;
    rob = {0, 0, 0, 0, 0};
    ene = {false, 0, 0, 0};
}

// Posición inicial uniforme dentro de radioInicio, con orientación aleatoria,
// con el cuerpo entero dentro del dohyo. No se coloca con un sensor de piso
// sobre la línea (lo prohíbe el reglamento y la calibración saldría mal).
// Posiciones de salida de los rounds (orientación global al azar y pequeñas
// imprecisiones al colocar los robots a mano)
inline void colocarRound() {
    const double phi = azar.entre(-PI, PI);
    const double th = phi + azar.normal() * 0.14;
    const double ux = mat::cos(th), uy = mat::sin(th);      // hacia donde mira nuestro robot
    const double rx = mat::sin(th), ry = -mat::cos(th);     // su derecha
    const double cx = azar.normal() * 0.015, cy = azar.normal() * 0.015;
    const double contacto = cfg.radioChoque + cfg.radioEnemigo + 0.01;
    double ox = 0, oy = 0;  // del centro a nuestro robot; el rival en el opuesto
    if (cfg.salida == 1) {
        ox = ux * contacto / 2; oy = uy * contacto / 2;
    } else if (cfg.salida == 2) {
        ox = -(cfg.ladoRival * rx * contacto - ux * 0.04) / 2;
        oy = -(cfg.ladoRival * ry * contacto - uy * 0.04) / 2;
    } else {
        // Pegados al borde, como en el reglamento del round 3 (centro del robot a
        // 5,5 cm del borde exterior: 33 cm del centro en un dohyo de 77 cm)
        ox = -ux * (cfg.radio - 0.055); oy = -uy * (cfg.radio - 0.055);
    }
    rob = {cx + ox, cy + oy, th, 0, 0};
    ene.presente = modo != Modo::Ninguno;
    if (ene.presente) {
        const double thEne = th + PI + azar.normal() * 0.14;
        ene = {true, cx - ox + azar.normal() * 0.01, cy - oy + azar.normal() * 0.01, thEne};
    }
}

inline void colocarAleatorio() {
    if (cfg.salida > 0) {
        colocarRound();
        return;
    }
    do {
        const double rIni = mat::sqrt(azar.uniforme()) * cfg.radioInicio, aIni = azar.entre(-PI, PI);
        rob = {rIni * mat::cos(aIni), rIni * mat::sin(aIni), azar.entre(-PI, PI), 0, 0};
    } while (sensorPisoSobreBlanco() || maxRadioCuerpo() > cfg.radio);

    ene.presente = modo != Modo::Ninguno;
    if (ene.presente) {
        // Sin tocarse al empezar (con contacto el choque los dispararía)
        do {
            const double r = mat::sqrt(azar.uniforme()) * (cfg.radio - 0.06);
            const double a = azar.entre(-PI, PI);
            ene = {true, r * mat::cos(a), r * mat::sin(a), azar.entre(-PI, PI)};
        } while (mat::hypot(ene.x - rob.x, ene.y - rob.y) < cfg.radioChoque + cfg.radioEnemigo + 0.02);
    }
}

// Un ciclo de loop() del firmware y la física correspondiente. Devuelve dt en s.
inline double paso() {
    costoLoopUs = 0;
    loop();
    const double dt = (costoLoopUs + cfg.sobrecostoLoopUs) / 1e6;
    resolverChoque(dt);
    avanzarRobot(dt);
    avanzarEnemigo(dt);
    tiempoUs += dt * 1e6;
    guardarPose();
    return dt;
}

// El robot cae cuando su centro de masa sale del dohyo
inline bool cayo() { return mat::hypot(rob.x, rob.y) > cfg.radio; }

}  // namespace sim

// ---------------------------------------------------------------- Arduino simulado

extern "C" {

void delay(unsigned long ms) {
    // Durante la espera el robot está quieto (motores detenidos en setup)
    const double finUs = sim::tiempoUs + ms * 1000.0;
    while (sim::tiempoUs < finUs) {
        sim::avanzarRobot(0.001);
        sim::tiempoUs += 1000;
    }
}

unsigned long millis(void) { return (unsigned long)(sim::tiempoUs / 1000); }
unsigned long micros(void) { return (unsigned long)sim::tiempoUs; }

int analogRead(uint8_t pin) {
    sim::costoLoopUs += 112;  // conversión ADC con prescaler 128 a 16 MHz
    if (pin == S_PISO_IZQ) return sim::lecturaPiso(sim::cfg.sensorPisoX, sim::cfg.sensorPisoY);
    if (pin == S_PISO_DER) return sim::lecturaPiso(sim::cfg.sensorPisoX, -sim::cfg.sensorPisoY);
    return 0;
}

int digitalRead(uint8_t pin) {
    sim::costoLoopUs += 4;
    // Módulo de arranque: el start ya está dado desde el principio del combate
    if ((int)pin == PIN_MODULO_ARRANQUE) {
        const bool enMarcha = !sim::corteArranque();
        return enMarcha == ModuloArranqueActivoAlto ? HIGH : LOW;
    }
    for (int i = 0; i < 3; i++) {
        if (pin == sim::cfg.pinesDip[i]) {
            const bool on = sim::cfg.dip >> i & 1;
            return on != sim::cfg.dipActivoBajo ? HIGH : LOW;
        }
    }
    return sim::sensorEnemigo(pin) != sim::cfg.enemigoInvertido ? HIGH : LOW;
}

void digitalWrite(uint8_t pin, uint8_t value) {
    sim::costoLoopUs += 4;
    if (pin < 32) sim::nivelPin[pin] = value ? HIGH : LOW;
}

void analogWrite(uint8_t pin, int value) {
    sim::costoLoopUs += 6;
    if (pin < 32) sim::pwmPin[pin] = constrain(value, 0, 255);
}

void pinMode(uint8_t, uint8_t) {}

}  // extern "C"

#endif
