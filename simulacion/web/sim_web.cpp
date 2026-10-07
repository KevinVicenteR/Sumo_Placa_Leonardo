// Simulador para la página web: el firmware real (src/) y la física de
// simulacion/fisica.h compilados a WebAssembly. La página coloca el robot y
// el enemigo, llama a sim_setup() (espera de 5 s y calibración) y luego
// avanza el combate con sim_avanzar().
//
// Para empezar un combate nuevo con el firmware recién reiniciado, la página
// crea otra instancia del módulo (memoria limpia, como tras un reset).

#include "../fisica.h"

#define EXPORTAR(nombre) extern "C" __attribute__((export_name(nombre)))

SerialSim Serial;

// Soporte mínimo de C++ sin biblioteca estándar
void operator delete(void*) noexcept {}
void operator delete(void*, unsigned long) noexcept {}
extern "C" void __cxa_pure_virtual() { __builtin_trap(); }

namespace {
double inicioCombateUs = 0;
}

// Sumo X3: 400 rpm, ruedas JSUMO de 3 cm, 340 g y pala delantera; el rival
// también lleva pala. Dohyo de 70 cm con línea de 2,5 cm.
EXPORTAR("sim_reiniciar") void sim_reiniciar(double semilla) {
    sim::cfg.rpm = 400;
    sim::cfg.parBloqueo = 0.8;
    sim::cfg.diametro = 0.03;
    sim::cfg.masa = 0.34;
    sim::cfg.mu = 0.9;
    sim::cfg.pala = 0.3;
    sim::cfg.palaRival = 0.3;
    sim::cfg.radio = 0.35;
    sim::cfg.borde = 0.025;
    sim::reiniciar((unsigned long long)semilla);
}

// Parámetros del robot y del entorno (ver sim::Config)
EXPORTAR("sim_fijar") void sim_fijar(int id, double v) {
    switch (id) {
    case 0: sim::cfg.rpm = v; break;
    case 1: sim::cfg.diametro = v; break;
    case 2: sim::cfg.masa = v; break;
    case 3: sim::cfg.bateria = v; break;
    case 4: sim::cfg.friccionCaja = v; break;
    case 5: sim::cfg.friccionGiro = v; break;
    case 6: sim::cfg.adcNegro = v; break;
    case 7: sim::cfg.adcBlanco = v; break;
    case 8: sim::cfg.adcFuera = v; break;
    case 9: sim::cfg.radioPared = v; break;
    case 10: sim::cfg.mu = v; break;
    case 11: sim::cfg.radioInicio = v; break;
    // Problemas externos (como en simulacion/bateria.py)
    case 12: sim::cfg.fantasmasPorSegundo = v; break;
    case 13: sim::cfg.saturacionesPorSegundo = v; break;
    case 14: sim::cfg.cortesArranquePorSegundo = v; break;
    case 15: sim::cfg.desgasteIzq = v; break;
    case 16: sim::cfg.desgasteDer = v; break;
    case 17: sim::cfg.agarreEnemigo = v; break;
    case 18: sim::cfg.masaEnemigo = v; break;
    case 19: sim::cfg.velAgresivo = v; break;
    case 20: sim::cfg.retardoPiso = v / 1000; break;  // ms
    default: break;
    }
}

// Round: posición de salida (0 libre, 1 espalda, 2 lado, 3 enfrentados), lado
// del rival en el round 2 (1 der, -1 izq) e interruptores DIP del robot
EXPORTAR("sim_round") void sim_round(int salida, int ladoRival, int dip) {
    sim::cfg.salida = salida;
    sim::cfg.ladoRival = ladoRival;
    sim::cfg.dip = dip;
}

EXPORTAR("sim_robot") void sim_robot(double x, double y, double th) {
    sim::rob = {x, y, th, 0, 0};
}

// modo: 0 sin enemigo, 1 quieto, 2 se mueve, 3 embiste, 4 flanquea
static sim::Modo modoDe(int modo) {
    return modo == 4 ? sim::Modo::Flanqueo : modo == 3 ? sim::Modo::Agresivo : modo == 2 ? sim::Modo::Errante
         : modo == 1 ? sim::Modo::Estatico : sim::Modo::Ninguno;
}

EXPORTAR("sim_enemigo") void sim_enemigo(int modo, double x, double y, double th) {
    sim::modo = modoDe(modo);
    sim::ene = {modo != 0, x, y, th};
}

// Colocación aleatoria igual que el simulador de consola (para comparar ambos)
EXPORTAR("sim_aleatorio") void sim_aleatorio(int modo) {
    sim::modo = modoDe(modo);
    sim::colocarAleatorio();
}

EXPORTAR("sim_setup") void sim_setup() {
    setup();
    inicioCombateUs = sim::tiempoUs;
}

// Avanza ciclos de loop() hasta el tiempo de combate dado (s).
// Devuelve 1 si el robot cayó y 2 si sacó al enemigo del dohyo.
EXPORTAR("sim_avanzar") int sim_avanzar(double hastaSegundos) {
    while ((sim::tiempoUs - inicioCombateUs) / 1e6 < hastaSegundos) {
        sim::paso();
        if (sim::cayo()) return 1;
        if (sim::enemigoFuera()) return 2;
    }
    return 0;
}

EXPORTAR("sim_tiempo") double sim_tiempo() { return (sim::tiempoUs - inicioCombateUs) / 1e6; }
EXPORTAR("sim_x") double sim_x() { return sim::rob.x; }
EXPORTAR("sim_y") double sim_y() { return sim::rob.y; }
EXPORTAR("sim_th") double sim_th() { return sim::rob.th; }
EXPORTAR("sim_ex") double sim_ex() { return sim::ene.x; }
EXPORTAR("sim_ey") double sim_ey() { return sim::ene.y; }
EXPORTAR("sim_eth") double sim_eth() { return sim::ene.th; }
EXPORTAR("sim_izq") int sim_izq() { return sim::comandoIzq(); }
EXPORTAR("sim_der") int sim_der() { return sim::comandoDer(); }
// Lo que ve cada sensor de enemigo, solo por geometría (sin reflejos ni
// saturación, para no alterar el sorteo del combate): 0 central, 1 a 45° izq,
// 2 a 45° der, 3 lateral izq, 4 lateral der
EXPORTAR("sim_vista") int sim_vista(int i) {
    const double fx = sim::cfg.largo / 2, ly = sim::cfg.ancho / 2;
    switch (i) {
    case 0: return sim::rayoVe(fx, 0, 0) ? 1 : 0;
    case 1: return sim::rayoVe(fx, ly * 0.6, sim::PI / 4) ? 1 : 0;
    case 2: return sim::rayoVe(fx, -ly * 0.6, -sim::PI / 4) ? 1 : 0;
    case 3: return sim::rayoVe(0, ly, sim::PI / 2) ? 1 : 0;
    case 4: return sim::rayoVe(0, -ly, -sim::PI / 2) ? 1 : 0;
    default: return 0;
    }
}
// La señal del módulo de arranque está en START (0 = cortada por una interferencia)
EXPORTAR("sim_start") int sim_start() { return sim::tiempoUs / 1e6 >= sim::finCorteArranque ? 1 : 0; }

EXPORTAR("sim_linea") int sim_linea() { return sim::sensorPisoSobreBlanco() ? 1 : 0; }
EXPORTAR("sim_cayo") int sim_cayo() { return sim::cayo() ? 1 : 0; }
EXPORTAR("sim_dentro") int sim_dentro() { return sim::maxRadioCuerpo() <= sim::cfg.radio ? 1 : 0; }
