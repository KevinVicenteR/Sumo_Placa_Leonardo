// Simulador de combate en dohyo circular (consola).
//
// Compila el firmware real (main.cpp y todo src/) contra un Arduino simulado
// (fisica.h): los pines de motores se traducen a velocidades de rueda y los
// pines de sensores se calculan a partir de la geometría del dohyo y del enemigo.
//
// Cada corrida se ejecuta en un proceso hijo (fork) para que el estado global
// del firmware (objetos en main.cpp) arranque limpio, igual que tras un reset.
//
// Salida: una línea CSV por corrida (ver imprimirCabecera()).

#include <cstdio>
#include <cstdlib>
#include <string>
#include <sys/wait.h>
#include <unistd.h>
#include "fisica.h"

SerialSim Serial;

namespace {

using namespace sim;

struct Resultado {
    bool cayo = false;
    double tCaida = -1;
    double margenMin = 1e9;      // m entre el centro del robot y el borde exterior
    double maxSalidaCuerpo = -1; // m que una esquina sobresale del borde
    int evasiones = 0;
    double tPrimerFrontal = -1;  // s hasta ver al enemigo de frente
    double fracFrontal = 0;      // fracción del tiempo con el enemigo de frente
    double distancia = 0;        // m recorridos
    bool gano = false;           // empujó al enemigo fuera del dohyo
    double tGano = -1;
};

void imprimirCabecera() {
    std::printf("semilla,modo,cayo,t_caida,margen_min_cm,max_salida_cuerpo_cm,evasiones,"
                "t_primer_frontal,frac_frontal,distancia_m,gano,t_gano\n");
}

const char* nombreModo(Modo m) {
    switch (m) {
    case Modo::Estatico: return "estatico";
    case Modo::Errante: return "errante";
    default: return "ninguno";
    }
}

Resultado correr(int semilla, FILE* trayectoria) {
    reiniciar(semilla);
    colocarAleatorio();
    setup();
    const double inicioUs = tiempoUs;

    Resultado res;
    bool lineaAntes = false;
    double tiempoFrontal = 0;
    double proximaMuestra = 0;

    while (true) {
        const double t = (tiempoUs - inicioUs) / 1e6;
        if (t >= cfg.duracion) break;

        const double xAntes = rob.x, yAntes = rob.y;
        const double dt = paso();
        res.distancia += mat::hypot(rob.x - xAntes, rob.y - yAntes);

        const bool linea = sensorPisoSobreBlanco();
        if (linea && !lineaAntes) res.evasiones++;
        lineaAntes = linea;

        if (rayoVeEnemigo(cfg.largo / 2, 0, 0)) {
            tiempoFrontal += dt;
            if (res.tPrimerFrontal < 0) res.tPrimerFrontal = t;
        }

        const double rCentro = mat::hypot(rob.x, rob.y);
        res.margenMin = minimo(res.margenMin, cfg.radio - rCentro);
        res.maxSalidaCuerpo = maximo(res.maxSalidaCuerpo, maxRadioCuerpo() - cfg.radio);

        if (trayectoria && t >= proximaMuestra) {
            proximaMuestra += 0.01;
            std::fprintf(trayectoria, "%.3f,%.4f,%.4f,%.4f,%.4f,%.4f,%d,%d,%d\n", t, rob.x, rob.y,
                         rob.th, ene.presente ? ene.x : 0.0, ene.presente ? ene.y : 0.0,
                         comandoIzq(), comandoDer(), linea ? 1 : 0);
        }

        if (cayo()) {
            res.cayo = true;
            res.tCaida = t;
            break;
        }
        if (enemigoFuera()) {
            res.gano = true;
            res.tGano = t;
            break;
        }
    }
    res.fracFrontal = tiempoFrontal / cfg.duracion;
    return res;
}

void uso() {
    std::fprintf(stderr,
                 "uso: sim [--modo ninguno|estatico|errante] [--n N] [--semilla S]\n"
                 "         [--dur s] [--vmax m/s] [--mu μ] [--tau s] [--rango m]\n"
                 "         [--rpm rpm --diam m --masa kg --par kg·cm]  (modelo de motor DC)\n"
                 "         [--bateria Vbat/Vmotor] [--friccion-caja f] [--friccion-giro f]\n"
                 "         [--adc-negro n --adc-blanco n --adc-fuera n] [--pared m] [--sensor-x m]\n"
                 "         [--inicio-r m] [--tray archivo.csv]  (trayectoria de la primera corrida)\n");
}

}  // namespace

int main(int argc, char** argv) {
    int n = 100;
    int semilla = 1;
    const char* rutaTray = nullptr;
    for (int i = 1; i < argc; i++) {
        const std::string a = argv[i];
        const char* v = (i + 1 < argc) ? argv[i + 1] : nullptr;
        if (!v) { uso(); return 1; }
        if (a == "--modo") {
            const std::string m = v;
            modo = m == "estatico" ? Modo::Estatico : m == "errante" ? Modo::Errante : Modo::Ninguno;
        } else if (a == "--n") n = std::atoi(v);
        else if (a == "--semilla") semilla = std::atoi(v);
        else if (a == "--dur") cfg.duracion = std::atof(v);
        else if (a == "--vmax") cfg.vmax = std::atof(v);
        else if (a == "--mu") cfg.mu = std::atof(v);
        else if (a == "--tau") cfg.tau = std::atof(v);
        else if (a == "--rpm") cfg.rpm = std::atof(v);
        else if (a == "--diam") cfg.diametro = std::atof(v);
        else if (a == "--masa") cfg.masa = std::atof(v);
        else if (a == "--par") cfg.parBloqueo = std::atof(v);
        else if (a == "--bateria") cfg.bateria = std::atof(v);
        else if (a == "--friccion-caja") cfg.friccionCaja = std::atof(v);
        else if (a == "--friccion-giro") cfg.friccionGiro = std::atof(v);
        else if (a == "--adc-negro") cfg.adcNegro = std::atof(v);
        else if (a == "--adc-blanco") cfg.adcBlanco = std::atof(v);
        else if (a == "--adc-fuera") cfg.adcFuera = std::atof(v);
        else if (a == "--pared") cfg.radioPared = std::atof(v);
        else if (a == "--sensor-x") cfg.sensorPisoX = std::atof(v);
        else if (a == "--inicio-r") cfg.radioInicio = std::atof(v);
        else if (a == "--rango") cfg.rangoEnemigo = std::atof(v);
        else if (a == "--masa-enemigo") cfg.masaEnemigo = std::atof(v);
        else if (a == "--agarre-enemigo") cfg.agarreEnemigo = std::atof(v);
        else if (a == "--tray") rutaTray = v;
        else { uso(); return 1; }
        i++;
    }

    imprimirCabecera();
    std::fflush(stdout);
    for (int i = 0; i < n; i++) {
        const int s = semilla + i;
        const pid_t pid = fork();
        if (pid == 0) {
            FILE* tray = (rutaTray && i == 0) ? std::fopen(rutaTray, "w") : nullptr;
            const Resultado r = correr(s, tray);
            if (tray) std::fclose(tray);
            std::printf("%d,%s,%d,%.3f,%.2f,%.2f,%d,%.3f,%.3f,%.2f,%d,%.3f\n", s, nombreModo(modo),
                        r.cayo ? 1 : 0, r.tCaida, r.margenMin * 100, r.maxSalidaCuerpo * 100,
                        r.evasiones, r.tPrimerFrontal, r.fracFrontal, r.distancia,
                        r.gano ? 1 : 0, r.tGano);
            std::fflush(stdout);
            _exit(0);
        }
        int estado = 0;
        waitpid(pid, &estado, 0);
    }
    return 0;
}
