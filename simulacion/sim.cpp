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

#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <sys/wait.h>
#include <unistd.h>
#include "fisica.h"
#include "ControlMovimiento.H"

extern ControlMovimiento controlMovimiento;

#ifdef ODOMETRIA_SIMULADA
// Encoders y giroscopio simulados: avance y giro reales desde la última lectura,
// con un 3 % de error (los empujones laterales no los mide ninguno de los dos)
bool leerOdometria(float& avance, float& giro) {
    static double x = 0, y = 0, th = 0;
    static bool primera = true;
    if (primera) { x = sim::rob.x; y = sim::rob.y; th = sim::rob.th; primera = false; }
    const double dx = sim::rob.x - x, dy = sim::rob.y - y;
    avance = (float)((dx * std::cos(th) + dy * std::sin(th)) * (1 + 0.03 * sim::azar.normal()));
    giro = (float)(std::remainder(sim::rob.th - th, 2 * sim::PI) * (1 + 0.03 * sim::azar.normal()));
    x = sim::rob.x; y = sim::rob.y; th = sim::rob.th;
    return true;
}
#endif

#ifdef ENTRENAMIENTO_POLITICA
// ---------------------------------------------------------------- entrenamiento
// El firmware pregunta qué acción tomar en cada estado (PoliticaAprendida); aquí
// se elige con exploración ε-greedy sobre la tabla Q cargada con --q, se
// mantiene cada decisión al menos 50 ms (si no, el robot temblaría) y se anotan
// las visitas para repartir la recompensa al acabar el combate.
#include <vector>
#include "PoliticaAprendida.H"

namespace entrenamiento {
double Q[politica::NumEstados][politica::NumAcciones];
double epsilon = 0.2;
double gammaSegundo = 0.8;      // descuento de la recompensa por segundo
unsigned long long estadoAzar = 1;
int estadoSostenido = -1, accionSostenida = 0;
double tSostenida = -1;
struct Visita { double t; int estado, accion; };
std::vector<Visita> visitas;

double azar() {
    estadoAzar ^= estadoAzar >> 12;
    estadoAzar ^= estadoAzar << 25;
    estadoAzar ^= estadoAzar >> 27;
    return ((estadoAzar * 0x2545F4914F6CDD1DULL) >> 11) * (1.0 / 9007199254740992.0);
}

int mejor(int e) {
    int m = 0;
    for (int a = 1; a < politica::NumAcciones; a++) if (Q[e][a] > Q[e][m]) m = a;
    return m;
}

bool cargar(const char* ruta) {
    FILE* f = std::fopen(ruta, "r");
    if (!f) return false;
    for (int e = 0; e < politica::NumEstados; e++)
        for (int a = 0; a < politica::NumAcciones; a++)
            if (std::fscanf(f, "%lf", &Q[e][a]) != 1) { std::fclose(f); return false; }
    std::fclose(f);
    return true;
}
}  // namespace entrenamiento

namespace politica {
int elegirAccionEntrenamiento(int estado, int) {
    using namespace entrenamiento;
    const double t = sim::tiempoUs / 1e6;
    if (estado != estadoSostenido || t - tSostenida >= 0.05) {
        accionSostenida = azar() < epsilon ? (int)(azar() * NumAcciones) % NumAcciones : mejor(estado);
        estadoSostenido = estado;
        tSostenida = t;
        visitas.push_back({t, estado, accionSostenida});
    }
    return accionSostenida;
}
}  // namespace politica
#endif

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
    char causa[16] = "-";        // qué hacía el robot al caer
    bool empujado = false;       // cayó con el enemigo encima
    double vueltas = 0;          // vueltas completas giradas en total
    int tirones = 0;             // saltos de más de 100 de PWM en la orden de un motor
};

// Clasifica la orden de motores: avance, retroceso, giro en el sitio, curva o freno
const char* tipoOrden(int izq, int der) {
    if (izq == 0 && der == 0) return "quieto";
    if (izq > 0 && der > 0) return izq == der ? "avance" : "curva";
    if (izq < 0 && der < 0) return izq == der ? "retroceso" : "freno";
    if ((izq > 0 && der < 0) || (izq < 0 && der > 0)) return "giro";
    return "pivote";
}

void imprimirCabecera() {
    std::printf("semilla,modo,cayo,t_caida,margen_min_cm,max_salida_cuerpo_cm,evasiones,"
                "t_primer_frontal,frac_frontal,distancia_m,gano,t_gano,causa,empujado,vueltas,tirones\n");
}

const char* nombreModo(Modo m) {
    switch (m) {
    case Modo::Estatico: return "estatico";
    case Modo::Errante: return "errante";
    case Modo::Agresivo: return "agresivo";
    default: return "ninguno";
    }
}

FILE* trayEstimador = nullptr;  // real frente a estimado (con --tray-estimador)

double inicioCombateUs = 0;

Resultado correr(int semilla, FILE* trayectoria) {
    reiniciar(semilla);
    colocarAleatorio();
#ifdef ENTRENAMIENTO_POLITICA
    entrenamiento::estadoAzar = semilla * 0x9E3779B97F4A7C15ULL + 7;
#endif
    setup();
    const double inicioUs = tiempoUs;
    inicioCombateUs = inicioUs;

    Resultado res;
    bool lineaAntes = false;
    double tiempoFrontal = 0;
    double proximaMuestra = 0;
    double ultimoContacto = -10;

    while (true) {
        const double t = (tiempoUs - inicioUs) / 1e6;
        if (t >= cfg.duracion) break;

        const double xAntes = rob.x, yAntes = rob.y, thAntes = rob.th;
        const int izqAntes = comandoIzq(), derAntes = comandoDer();
        const double dt = paso();
        res.distancia += mat::hypot(rob.x - xAntes, rob.y - yAntes);
        res.vueltas += mat::fabs(rob.th - thAntes) / (2 * PI);
        if (std::abs(comandoIzq() - izqAntes) > 100 || std::abs(comandoDer() - derAntes) > 100) res.tirones++;

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
            std::fprintf(trayectoria, "%.3f,%.4f,%.4f,%.4f,%.4f,%.4f,%d,%d,%d,%.3f,%.3f,%.2f\n", t, rob.x,
                         rob.y, rob.th, ene.presente ? ene.x : 0.0, ene.presente ? ene.y : 0.0,
                         comandoIzq(), comandoDer(), linea ? 1 : 0, (rob.vl + rob.vr) / 2, rob.vlat,
                         rob.fExt);
            if (trayEstimador) {
                const EstimadorBorde& e = controlMovimiento.estimadorBorde();
                std::fprintf(trayEstimador, "%.3f,%.4f,%.4f,%.4f,%.4f,%.4f,%.4f,%.4f,%.4f,%d\n", t, rob.x, rob.y,
                             rob.th, e.x(), e.y(), e.rumbo(), e.incertidumbrePosicion(),
                             e.incertidumbreRumbo(), controlMovimiento.limiteAvanceActual());
            }
        }

        // Contacto reciente con el enemigo: la caída pudo ser por empujón
        if (ene.presente && mat::hypot(ene.x - rob.x, ene.y - rob.y) < cfg.radioChoque + cfg.radioEnemigo + 0.005)
            ultimoContacto = t;
        if (cayo()) {
            res.cayo = true;
            res.tCaida = t;
            std::snprintf(res.causa, sizeof res.causa, "%s", tipoOrden(comandoIzq(), comandoDer()));
            res.empujado = t - ultimoContacto < 0.3;
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
                 "uso: sim [--modo ninguno|estatico|errante|agresivo] [--n N] [--semilla S]\n"
                 "         [--dur s] [--vmax m/s] [--mu μ] [--tau s] [--rango m]\n"
                 "         [--rpm rpm --diam m --masa kg --par kg·cm]  (modelo de motor DC)\n"
                 "         [--bateria Vbat/Vmotor] [--friccion-caja f] [--friccion-giro f]\n"
                 "         [--adc-negro n --adc-blanco n --adc-fuera n] [--pared m] [--sensor-x m]\n"
                 "         [--inicio-r m] [--retardo-piso ms] [--mancha m]\n"
                 "         [--salida espalda|lado|frente] [--rival-lado der|izq] [--dip N]\n"
                 "                  (posición de salida de cada round; DIP: bit0=DIP1, bit1=DIP2, bit2=DIP3)\n"
                 "         [--fantasmas n]  (detecciones falsas por segundo y sensor)\n"
                 "         [--driver tb6612|l298n]  (con L298N, PWM a 0 deja el motor libre)\n"
                 "         [--enemigo-invertido 1]  (sensores de enemigo que dan LOW al detectar)\n"
                 "         [--radio m --borde m]  (por defecto 0.385 y 0.025: minisumo reglamentario)\n"
                 "         [--tray archivo.csv]  (trayectoria de la primera corrida)\n");
}

}  // namespace

int main(int argc, char** argv) {
    int n = 100;
    int semilla = 1;
    const char* rutaTray = nullptr;
    const char* rutaTrayEstimador = nullptr;
    for (int i = 1; i < argc; i++) {
        const std::string a = argv[i];
        const char* v = (i + 1 < argc) ? argv[i + 1] : nullptr;
        if (!v) { uso(); return 1; }
        if (a == "--modo") {
            const std::string m = v;
            modo = m == "estatico" ? Modo::Estatico : m == "errante" ? Modo::Errante
                 : m == "agresivo" ? Modo::Agresivo : Modo::Ninguno;
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
        else if (a == "--vel-agresivo") cfg.velAgresivo = std::atof(v);
        else if (a == "--retardo-piso") cfg.retardoPiso = std::atof(v) / 1000;  // ms
        else if (a == "--mancha") cfg.manchaSensor = std::atof(v);
        else if (a == "--radio") cfg.radio = std::atof(v);
        else if (a == "--enemigo-invertido") cfg.enemigoInvertido = std::atoi(v) != 0;
        else if (a == "--driver") cfg.driverL298 = std::string(v) == "l298n";
        else if (a == "--salida") {
            const std::string m = v;
            cfg.salida = m == "espalda" ? 1 : m == "lado" ? 2 : m == "frente" ? 3 : 0;
        }
        else if (a == "--rival-lado") cfg.ladoRival = std::string(v) == "izq" ? -1 : 1;
        else if (a == "--dip") cfg.dip = std::atoi(v);
        else if (a == "--fantasmas") cfg.fantasmasPorSegundo = std::atof(v);
        else if (a == "--borde") cfg.borde = std::atof(v);
        else if (a == "--tray") rutaTray = v;
#ifdef ENTRENAMIENTO_POLITICA
        else if (a == "--q") { if (!entrenamiento::cargar(v)) { std::fprintf(stderr, "no se pudo leer %s\n", v); return 1; } }
        else if (a == "--epsilon") entrenamiento::epsilon = std::atof(v);
#endif
        else if (a == "--tray-estimador") rutaTrayEstimador = v;
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
            if (rutaTrayEstimador && i == 0) trayEstimador = std::fopen(rutaTrayEstimador, "w");
            const Resultado r = correr(s, tray);
            if (tray) std::fclose(tray);
            std::printf("%d,%s,%d,%.3f,%.2f,%.2f,%d,%.3f,%.3f,%.2f,%d,%.3f,%s,%d,%.1f,%d\n", s, nombreModo(modo),
                        r.cayo ? 1 : 0, r.tCaida, r.margenMin * 100, r.maxSalidaCuerpo * 100,
                        r.evasiones, r.tPrimerFrontal, r.fracFrontal, r.distancia,
                        r.gano ? 1 : 0, r.tGano, r.causa, r.empujado ? 1 : 0, r.vueltas, r.tirones);
#ifdef ENTRENAMIENTO_POLITICA
            {
                // Retorno de Monte Carlo de cada visita: recompensa final descontada
                using namespace entrenamiento;
                const double recompensa = r.gano ? 1.0 : r.cayo ? -1.0 : 0.0;
                const double fin = inicioCombateUs / 1e6 + (r.cayo ? r.tCaida : r.gano ? r.tGano : cfg.duracion);
                static double suma[politica::NumEstados][politica::NumAcciones];
                static int n[politica::NumEstados][politica::NumAcciones];
                for (const Visita& v : visitas) {
                    suma[v.estado][v.accion] += recompensa * std::pow(gammaSegundo, fin - v.t);
                    n[v.estado][v.accion]++;
                }
                std::printf("Q");
                for (int e = 0; e < politica::NumEstados; e++)
                    for (int a = 0; a < politica::NumAcciones; a++)
                        if (n[e][a]) std::printf(" %d:%d:%.5f:%d", e, a, suma[e][a], n[e][a]);
                std::printf("\n");
            }
#endif
            std::fflush(stdout);
            _exit(0);
        }
        int estado = 0;
        waitpid(pid, &estado, 0);
    }
    return 0;
}
