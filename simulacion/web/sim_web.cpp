#include "../fisica.h"

#define EXPORTAR(nombre) extern "C" __attribute__((export_name(nombre)))

SerialSim Serial;

void operator delete(void*) noexcept {}
void operator delete(void*, unsigned long) noexcept {}
extern "C" void __cxa_pure_virtual() { __builtin_trap(); }

namespace {
double inicioCombateUs = 0;
}

EXPORTAR("sim_reiniciar") void sim_reiniciar(double semilla) {
    sim::cfg.rpm = 750;
    sim::cfg.diametro = 0.03;
    sim::cfg.masa = 0.3;
    sim::cfg.mu = 1.0;
    sim::reiniciar((unsigned long long)semilla);
}

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
    default: break;
    }
}

EXPORTAR("sim_robot") void sim_robot(double x, double y, double th) {
    sim::rob = {x, y, th, 0, 0};
}

static sim::Modo modoDe(int modo) {
    return modo == 3 ? sim::Modo::Agresivo : modo == 2 ? sim::Modo::Errante
         : modo == 1 ? sim::Modo::Estatico : sim::Modo::Ninguno;
}

EXPORTAR("sim_enemigo") void sim_enemigo(int modo, double x, double y, double th) {
    sim::modo = modoDe(modo);
    sim::ene = {modo != 0, x, y, th};
}

EXPORTAR("sim_aleatorio") void sim_aleatorio(int modo) {
    sim::modo = modoDe(modo);
    sim::colocarAleatorio();
}

EXPORTAR("sim_setup") void sim_setup() {
    setup();
    inicioCombateUs = sim::tiempoUs;
}

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
EXPORTAR("sim_linea") int sim_linea() { return sim::sensorPisoSobreBlanco() ? 1 : 0; }
EXPORTAR("sim_cayo") int sim_cayo() { return sim::cayo() ? 1 : 0; }
EXPORTAR("sim_dentro") int sim_dentro() { return sim::maxRadioCuerpo() <= sim::cfg.radio ? 1 : 0; }
