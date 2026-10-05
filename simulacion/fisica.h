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
    double radio = 0.385;
    double borde = 0.025;
    double largo = 0.10;
    double ancho = 0.10;
    double trocha = 0.085;
    double sensorPisoX = 0.045;
    double sensorPisoY = 0.040;
    double vmax = 0.8;
    double tau = 0.05;
    double mu = 0.9;
    double rpm = 0;
    double diametro = 0.03;
    double masa = 0.3;
    double parBloqueo = 0.6;
    double bateria = 1.0;
    double friccionCaja = 0.15;
    double rodadura = 0.03;
    double friccionGiro = 0.5;
    double brazoGiro = 0.012;
    double adcNegro = 900;
    double adcBlanco = 100;
    double adcFuera = 1000;
    double manchaSensor = 0.003;
    double radioPared = 0;
    double radioInicio = 0.12;
    double rangoEnemigo = 0.40;
    bool enemigoInvertido = false;
    bool driverL298 = false;
    int salida = 0;
    int ladoRival = 1;
    int dip = 0;
    uint8_t pinesDip[3] = {5, 6, 7};
    bool dipActivoBajo = true;
    double fantasmasPorSegundo = 0;
    double radioEnemigo = 0.05;
    double velEnemigo = 0.25;
    double velAgresivo = 0.6;
    double giroAgresivo = 6.0;
    double masaEnemigo = 0.3;
    double agarreEnemigo = 0.8;
    double radioChoque = 0.055;
    double rigidezChoque = 3000;
    double amortiguaChoque = 25;
    double duracion = 30.0;
    double ruidoPiso = 15.0;
    double retardoPiso = 0;
    double sobrecostoLoopUs = 20;
};

enum class Modo { Ninguno, Estatico, Errante, Agresivo };

struct Robot {
    double x, y, th;
    double vl, vr;
    double vlat = 0;
    double fExt = 0;
};

struct Enemigo {
    bool presente;
    double x, y, th;
    double vx = 0, vy = 0;
};

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

inline double valorPiso(double wx, double wy) {
    const double r = mat::hypot(wx, wy);
    if (r <= cfg.radio - cfg.borde) return cfg.adcNegro;
    if (r <= cfg.radio) return cfg.adcBlanco;
    return cfg.adcFuera;
}

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

inline bool rayoVePared(double rx, double ry, double ang) {
    if (cfg.radioPared <= 0) return false;
    double ox, oy;
    aMundo(rx, ry, ox, oy);
    const double ux = mat::cos(rob.th + ang), uy = mat::sin(rob.th + ang);
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

inline bool sensorEnemigo(uint8_t pin) {
    const double fx = cfg.largo / 2, ly = cfg.ancho / 2;
    const uint8_t pines[5] = {S_FRONT_CEN, S_FRONT_IZQ, S_FRONT_DER, S_LAT_IZQ, S_LAT_DER};
    for (int i = 0; i < 5; i++) {
        if (pin == pines[i] && fantasma(i)) return true;
    }
    if (pin == S_FRONT_CEN) return rayoVe(fx, 0, 0);
    if (pin == S_FRONT_IZQ) return rayoVe(fx, ly * 0.6, PI / 4);
    if (pin == S_FRONT_DER) return rayoVe(fx, -ly * 0.6, -PI / 4);
    if (pin == S_LAT_IZQ) return rayoVe(0, ly, PI / 2);
    if (pin == S_LAT_DER) return rayoVe(0, -ly, -PI / 2);
    return false;
}

inline int comandoMotor(uint8_t in1, uint8_t in2, uint8_t pwm) {
    if (nivelPin[in1] == nivelPin[in2]) return 0;
    return nivelPin[in1] == HIGH ? pwmPin[pwm] : -pwmPin[pwm];
}

enum class Puente { Conduce, Frena, Libre };
inline Puente estadoPuente(uint8_t pwm) {
    return pwmPin[pwm] == 0 ? Puente::Libre : Puente::Conduce;
}

inline int comandoIzq() { return comandoMotor(MA1A, MA2A, PWMA); }
inline int comandoDer() { return comandoMotor(MA1B, MA2B, PWMB); }

inline double conFriccion(double fuerza, double vel, double f) {
    if (mat::fabs(vel) < 1e-4) {
        if (mat::fabs(fuerza) <= f) return 0;
        return fuerza - (fuerza > 0 ? f : -f);
    }
    return fuerza - (vel > 0 ? f : -f);
}

inline double fuerzaRueda(int comando, double vRueda, Puente puente = Puente::Conduce) {
    const double u = (puente == Puente::Conduce ? comando : 0) / 255.0 * cfg.bateria;
    const double v0 = cfg.rpm / 60.0 * PI * cfg.diametro;
    const double fuerzaBloqueo = cfg.parBloqueo * 0.0981 / (cfg.diametro / 2);
    double motor = fuerzaBloqueo * (u - vRueda / v0);
    if (puente == Puente::Libre) motor = 0;
    if (puente == Puente::Conduce && cfg.driverL298 && motor * u < 0) motor = 0;
    double f = conFriccion(motor, vRueda, cfg.friccionCaja * fuerzaBloqueo);
    const double adherencia = cfg.mu * cfg.masa / 2 * 9.81;
    return constrain(f, -adherencia, adherencia);
}

inline void avanzarCuerpoDC(int cmdIzq, int cmdDer, double dt) {
    const double b = cfg.trocha / 2;
    double v = (rob.vl + rob.vr) / 2;
    double w = (rob.vr - rob.vl) / cfg.trocha;
    const double fi = fuerzaRueda(cmdIzq, rob.vl, estadoPuente(PWMA));
    const double fd = fuerzaRueda(cmdDer, rob.vr, estadoPuente(PWMB));
    const double peso = cfg.masa * 9.81;
    const double inercia = cfg.masa * (cfg.largo * cfg.largo + cfg.ancho * cfg.ancho) / 12;
    const double fuerza = conFriccion(fi + fd + rob.fExt, v, cfg.rodadura * peso);
    const double par = conFriccion((fd - fi) * b, w, cfg.friccionGiro * peso * cfg.brazoGiro);
    double vNueva = v + fuerza / cfg.masa * dt;
    double wNueva = w + par / inercia * dt;
    if (v != 0 && vNueva * v < 0 && mat::fabs(fi + fd + rob.fExt) < cfg.rodadura * peso) vNueva = 0;
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

inline void avanzarEnemigo(double dt) {
    if (!ene.presente) return;
    double vdx = 0, vdy = 0;
    if (modo == Modo::Errante) {
        ene.th += azar.normal() * 3.0 * mat::sqrt(dt);
        const double r = mat::hypot(ene.x, ene.y);
        if (r > cfg.radio - 0.08) {
            const double haciaCentro = mat::atan2(-ene.y, -ene.x);
            double diff = mat::remainder(haciaCentro - ene.th, 2 * PI);
            ene.th += diff * minimo(1.0, 8.0 * dt);
        }
        vdx = cfg.velEnemigo * mat::cos(ene.th);
        vdy = cfg.velEnemigo * mat::sin(ene.th);
    } else if (modo == Modo::Agresivo) {
        const double haciaRobot = mat::atan2(rob.y - ene.y, rob.x - ene.x);
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
        const double maxDv = cfg.agarreEnemigo * 9.81 * dt;
        const double f = dv > maxDv ? maxDv / dv : 1.0;
        ene.vx += dvx * f;
        ene.vy += dvy * f;
    }
    ene.x += ene.vx * dt;
    ene.y += ene.vy * dt;
}

inline void resolverChoque(double dt) {
    rob.fExt = 0;
    double fLat = 0;
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
            if (f < 0) f = 0;
            ene.vx += f * nx / cfg.masaEnemigo * dt;
            ene.vy += f * ny / cfg.masaEnemigo * dt;
            rob.fExt = -f * (nx * c + ny * s);
            fLat = -f * (-nx * s + ny * c);
        }
    }
    const double agarre = cfg.mu * cfg.masa * 9.81;
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

inline void colocarRound() {
    const double phi = azar.entre(-PI, PI);
    const double th = phi + azar.normal() * 0.14;
    const double ux = mat::cos(th), uy = mat::sin(th);
    const double rx = mat::sin(th), ry = -mat::cos(th);
    const double cx = azar.normal() * 0.015, cy = azar.normal() * 0.015;
    const double contacto = cfg.radioChoque + cfg.radioEnemigo + 0.01;
    double ox = 0, oy = 0;
    if (cfg.salida == 1) {
        ox = ux * contacto / 2; oy = uy * contacto / 2;
    } else if (cfg.salida == 2) {
        ox = -(cfg.ladoRival * rx * contacto - ux * 0.04) / 2;
        oy = -(cfg.ladoRival * ry * contacto - uy * 0.04) / 2;
    } else {
        ox = -ux * 0.33; oy = -uy * 0.33;
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
        do {
            const double r = mat::sqrt(azar.uniforme()) * (cfg.radio - 0.06);
            const double a = azar.entre(-PI, PI);
            ene = {true, r * mat::cos(a), r * mat::sin(a), azar.entre(-PI, PI)};
        } while (mat::hypot(ene.x - rob.x, ene.y - rob.y) < cfg.radioChoque + cfg.radioEnemigo + 0.02);
    }
}

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

inline bool cayo() { return mat::hypot(rob.x, rob.y) > cfg.radio; }

}

extern "C" {
void delay(unsigned long ms) {
    const double finUs = sim::tiempoUs + ms * 1000.0;
    while (sim::tiempoUs < finUs) {
        sim::avanzarRobot(0.001);
        sim::tiempoUs += 1000;
    }
}

unsigned long millis(void) { return (unsigned long)(sim::tiempoUs / 1000); }
unsigned long micros(void) { return (unsigned long)sim::tiempoUs; }

int analogRead(uint8_t pin) {
    sim::costoLoopUs += 112;
    if (pin == S_PISO_IZQ) return sim::lecturaPiso(sim::cfg.sensorPisoX, sim::cfg.sensorPisoY);
    if (pin == S_PISO_DER) return sim::lecturaPiso(sim::cfg.sensorPisoX, -sim::cfg.sensorPisoY);
    return 0;
}

int digitalRead(uint8_t pin) {
    sim::costoLoopUs += 4;
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

}

#endif
