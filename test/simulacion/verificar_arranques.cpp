#include <cstdio>
#include <cstdint>
#include <type_traits>
#include "estrategia/EstrategiaReglas.h"
#include "estrategia/EstrategiaAprendida.h"
#include "movimiento/ControlMovimiento.h"
#include "Parametros.h"
#ifdef PORT_MODO4
#include "estrategia/EstrategiaCompetencia.h"
#endif
extern unsigned long g_millis;
class Motor: public IMotor { public: int izq=0,der=0; void mover(int a,int b) override {izq=a;der=b;} void detener() override {mover(0,0);} };
int main() {
    using Clasica=std::conditional<UsarPoliticaAprendida,EstrategiaAprendida,EstrategiaReglas>::type;
    uint32_t rng=872342;
    for (int rutina=1;rutina<=3;rutina++) for (int lado=-1;lado<=1;lado+=2) {
        ControlMovimiento c; Clasica clasica; Motor m;
#ifdef PORT_MODO4
        EstrategiaCompetencia estrategia(clasica,c);
#else
        auto& estrategia=clasica;
#endif
        for (int pelea=0;pelea<5;pelea++) {
            c.reiniciar(); c.iniciarRutina(rutina,lado);
            unsigned bits=0;
            for(int i=0;i<5000;i++) {
                if(i%30==0) { rng=rng*1664525u+1013904223u; bits=rng>>24; }
                if(i<1000) {
                    if(pelea==0) bits=0;
                    if(pelea==1) bits=112;
                    if(pelea==2) bits=8;
                    if(pelea==3) bits=i<200?0:3;
                    if(pelea==4) bits=i<200?0:16;
                }
                g_millis=1000+pelea*100000+i*10;
                LecturasSensores l{bool(bits&1),bool(bits&2),bool(bits&4),bool(bits&8),bool(bits&16),bool(bits&32),bool(bits&64),bool(bits&128)};
                const auto d=estrategia.decidir(l);c.ejecutar(d,m,g_millis);
                std::printf("%d,%d,%d,%d,%d,%d,%d,%d,%u,%d\n",rutina,lado,pelea,i,(int)d.tipo,m.izq,m.der,c.rutinaActual(),c.evasiones(),c.limiteAvanceActual());
            }
        }
    }
}
