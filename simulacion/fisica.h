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
#include "Pines.H"

void setup();
void loop();

namespace sim {

constexpr double PI = 3.14159265358979323846;

struct Config {
    // Dohyo
    double radio = 0.35;          // m (70 cm de diámetro, borde incluido)
    double borde = 0.01;          // m de línea blanca
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
    // Enemigo
    double radioEnemigo = 0.05;   // m
    double velEnemigo = 0.25;     // m/s en modo errante
    // Choque con el enemigo (0 en masaEnemigo = fantasma, el robot lo atraviesa)
    double masaEnemigo = 0.3;     // kg
    double agarreEnemigo = 0.8;   // su resistencia a ser empujado (μ de sus ruedas)
    double radioChoque = 0.055;   // m, radio de choque de nuestro robot
    // Simulación
    double duracion = 30.0;       // s de combate (tras los 5 s reglamentarios)
    double ruidoPiso = 15.0;      // desviación estándar del ADC
    double sobrecostoLoopUs = 20; // µs de loop() además de las lecturas
};

enum class Modo { Ninguno, Estatico, Errante };

struct Robot {
    double x, y, th;  // posición del eje de ruedas y orientación (rad, antihorario)
    double vl, vr;    // velocidad real de cada rueda
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
inline Aleatorio azar;

inline double tiempoUs = 0;
inline double costoLoopUs = 0;
inline uint8_t nivelPin[32];
inline int pwmPin[32];

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
    double wx, wy;
    aMundo(rx, ry, wx, wy);
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

inline bool sensorEnemigo(uint8_t pin) {
    const double fx = cfg.largo / 2, ly = cfg.ancho / 2;
    if (pin == S_FRONT_CEN) return rayoVe(fx, 0, 0);
    if (pin == S_FRONT_IZQ) return rayoVe(fx, ly * 0.6, PI / 4);
    if (pin == S_FRONT_DER) return rayoVe(fx, -ly * 0.6, -PI / 4);
    if (pin == S_LAT_IZQ) return rayoVe(0, ly, PI / 2);
    if (pin == S_LAT_DER) return rayoVe(0, -ly, -PI / 2);
    return false;
}

// ---------------------------------------------------------------- física

inline int comandoMotor(uint8_t in1, uint8_t in2, uint8_t pwm) {
    const int magnitud = pwmPin[pwm];
    if (nivelPin[in1] == nivelPin[in2]) return 0;  // freno / libre
    return nivelPin[in1] == HIGH ? magnitud : -magnitud;
}

inline int comandoIzq() { return comandoMotor(MA1A, MA2A, PWMA); }
inline int comandoDer() { return comandoMotor(MA1B, MA2B, PWMB); }

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
// limitada por la adherencia de cada rueda.
inline double fuerzaRueda(int comando, double vRueda) {
    const double u = comando / 255.0 * cfg.bateria;
    const double v0 = cfg.rpm / 60.0 * PI * cfg.diametro;
    const double fuerzaBloqueo = cfg.parBloqueo * 0.0981 / (cfg.diametro / 2);
    double f = conFriccion(fuerzaBloqueo * (u - vRueda / v0), vRueda, cfg.friccionCaja * fuerzaBloqueo);
    const double adherencia = cfg.mu * cfg.masa / 2 * 9.81;
    return constrain(f, -adherencia, adherencia);
}

// Cuerpo rígido: avance v y giro w con rodadura y roce lateral al girar
inline void avanzarCuerpoDC(int cmdIzq, int cmdDer, double dt) {
    const double b = cfg.trocha / 2;
    double v = (rob.vl + rob.vr) / 2;
    double w = (rob.vr - rob.vl) / cfg.trocha;
    const double fi = fuerzaRueda(cmdIzq, rob.vl);
    const double fd = fuerzaRueda(cmdDer, rob.vr);
    const double peso = cfg.masa * 9.81;
    const double inercia = cfg.masa * (cfg.largo * cfg.largo + cfg.ancho * cfg.ancho) / 12;
    const double fuerza = conFriccion(fi + fd, v, cfg.rodadura * peso);
    const double par = conFriccion((fd - fi) * b, w, cfg.friccionGiro * peso * cfg.brazoGiro);
    double vNueva = v + fuerza / cfg.masa * dt;
    double wNueva = w + par / inercia * dt;
    // La fricción frena hasta cero, no invierte el movimiento
    if (v != 0 && vNueva * v < 0 && mat::fabs(fi + fd) < cfg.rodadura * peso) vNueva = 0;
    if (w != 0 && wNueva * w < 0 && mat::fabs((fd - fi) * b) < cfg.friccionGiro * peso * cfg.brazoGiro) wNueva = 0;
    rob.vl = vNueva - wNueva * b;
    rob.vr = vNueva + wNueva * b;
}

inline void avanzarRueda(double& v, int comando, double dt) {
    const double objetivo = comando / 255.0 * cfg.vmax;
    double dv = (objetivo - v) * dt / cfg.tau;
    const double maxDv = cfg.mu * 9.81 * dt;
    dv = constrain(dv, -maxDv, maxDv);
    v += dv;
}

inline void avanzarRobot(double dt) {
    if (cfg.rpm > 0) {
        avanzarCuerpoDC(comandoIzq(), comandoDer(), dt);
    } else {
        avanzarRueda(rob.vl, comandoIzq(), dt);
        avanzarRueda(rob.vr, comandoDer(), dt);
    }
    const double v = (rob.vl + rob.vr) / 2;
    const double w = (rob.vr - rob.vl) / cfg.trocha;
    rob.x += v * mat::cos(rob.th) * dt;
    rob.y += v * mat::sin(rob.th) * dt;
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
    }
    if (cfg.masaEnemigo <= 0) {
        ene.vx = vdx;
        ene.vy = vdy;
    } else {
        const double dvx = vdx - ene.vx, dvy = vdy - ene.vy;
        const double dv = mat::hypot(dvx, dvy);
        const double maxDv = cfg.agarreEnemigo * 9.81 * dt;
        const double f = dv > maxDv ? maxDv / dv : 1.0;
        ene.vx += dvx * f;
        ene.vy += dvy * f;
    }
    ene.x += ene.vx * dt;
    ene.y += ene.vy * dt;
}

// Choque inelástico entre los dos robots (como círculos): se separan y comparten
// la velocidad a lo largo de la línea que une sus centros
inline void resolverChoque() {
    if (!ene.presente || cfg.masaEnemigo <= 0) return;
    const double dx = ene.x - rob.x, dy = ene.y - rob.y;
    const double d = mat::hypot(dx, dy);
    const double minima = cfg.radioChoque + cfg.radioEnemigo;
    if (d >= minima || d < 1e-9) return;
    const double nx = dx / d, ny = dy / d;
    const double mr = cfg.masa, me = cfg.masaEnemigo;
    const double solape = minima - d;
    rob.x -= nx * solape * me / (mr + me);
    rob.y -= ny * solape * me / (mr + me);
    ene.x += nx * solape * mr / (mr + me);
    ene.y += ny * solape * mr / (mr + me);

    const double v = (rob.vl + rob.vr) / 2;
    const double c = mat::cos(rob.th) * nx + mat::sin(rob.th) * ny;  // rumbo · normal
    const double vrn = v * c;
    const double ven = ene.vx * nx + ene.vy * ny;
    if (vrn <= ven) return;  // ya se separan
    const double comun = (mr * vrn + me * ven) / (mr + me);
    ene.vx += (comun - ven) * nx;
    ene.vy += (comun - ven) * ny;
    // El robot solo puede cambiar su velocidad hacia adelante (las ruedas no derrapan de lado)
    const double dv = (comun - vrn) * c;
    rob.vl += dv;
    rob.vr += dv;
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
    rob = {0, 0, 0, 0, 0};
    ene = {false, 0, 0, 0};
}

// Posición inicial uniforme dentro de radioInicio, con orientación aleatoria,
// con el cuerpo entero dentro del dohyo. No se coloca con un sensor de piso
// sobre la línea (lo prohíbe el reglamento y la calibración saldría mal).
inline void colocarAleatorio() {
    do {
        const double rIni = mat::sqrt(azar.uniforme()) * cfg.radioInicio, aIni = azar.entre(-PI, PI);
        rob = {rIni * mat::cos(aIni), rIni * mat::sin(aIni), azar.entre(-PI, PI), 0, 0};
    } while (sensorPisoSobreBlanco() || maxRadioCuerpo() > cfg.radio);

    ene.presente = modo != Modo::Ninguno;
    if (ene.presente) {
        const double r = mat::sqrt(azar.uniforme()) * (cfg.radio - 0.06);
        const double a = azar.entre(-PI, PI);
        ene = {true, r * mat::cos(a), r * mat::sin(a), azar.entre(-PI, PI)};
    }
}

// Un ciclo de loop() del firmware y la física correspondiente. Devuelve dt en s.
inline double paso() {
    costoLoopUs = 0;
    loop();
    const double dt = (costoLoopUs + cfg.sobrecostoLoopUs) / 1e6;
    avanzarRobot(dt);
    avanzarEnemigo(dt);
    resolverChoque();
    tiempoUs += dt * 1e6;
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
    return sim::sensorEnemigo(pin) ? HIGH : LOW;
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
