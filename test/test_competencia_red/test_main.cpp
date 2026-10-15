#include <unity.h>
#include <vector>
#include <cstring>
#include "estrategia/EstrategiaCompetencia.h"
#include "estrategia/EstrategiaReglas.h"
#include "movimiento/RedControl.h"
#include "ParametrosModo4.h"
extern unsigned long g_millis;
static std::vector<int8_t> logica, motores;
void setUp() {
    logica.assign(redlogica::Parametros, redlogica::Parametros + redlogica::numParametros());
    motores.assign(redcontrol::Parametros, redcontrol::Parametros + redcontrol::numParametros());
    g_millis = 1000;
}
void tearDown() {
    std::memcpy(redlogica::Parametros, logica.data(), logica.size());
    std::memcpy(redcontrol::Parametros, motores.data(), motores.size());
}
class Motor: public IMotor {
public: int izq=0,der=0;
    void mover(int i,int d) override { izq=i;der=d; }
    void detener() override { mover(0,0); }
};
static void forzarAccion(int k) {
    const int n=redlogica::numParametros(), e=redlogica::NumEntradasRed;
    const int h=(n-redlogica::NumAcciones)/(e+1+redlogica::NumAcciones);
    std::memset(redlogica::Parametros,0,n);
    redlogica::Parametros[h*(e+1)+k*(h+1)+h]=64;
}
static LecturasSensores frontal() { return {false,false,false,false,true,true,true,false}; }
void test_arranques_protegidos_y_mejoras_solo_al_terminar() {
    forzarAccion(8);
    for(int modo=1;modo<=3;++modo) {
        ControlMovimiento c(0,0); EstrategiaReglas clasica;
        EstrategiaCompetencia estrategia(clasica,c); Motor m;
        c.iniciarRutina(modo,1);
        TEST_ASSERT_FALSE(c.mejorasActivas());
        TEST_ASSERT_EQUAL_INT((int)clasica.decidir(frontal()).tipo,(int)estrategia.decidir(frontal()).tipo);
        c.ejecutar({TipoAccion::EvadirBordeAmbos},m,1000);
        TEST_ASSERT_EQUAL_INT(0,c.rutinaActual());
        TEST_ASSERT_TRUE(c.mejorasActivas());
        const bool usaPolitica = UsarRedLogica && (modo != 1 || UsarRedLogicaModo1) && (modo != 3 || UsarRedLogicaModo3);
        TEST_ASSERT_EQUAL_INT((int)(usaPolitica ? TipoAccion::EsquivarIzq : TipoAccion::AtaqueFrontal),
                              (int)estrategia.decidir(frontal()).tipo);
    }
}
void test_modo4_mejora_desde_inicio_y_no_se_pierde_al_cancelar_rutina() {
    forzarAccion(8); ControlMovimiento c(0,0); EstrategiaReglas clasica;
    EstrategiaCompetencia estrategia(clasica,c); Motor m;
    c.iniciarRutina(4,-1);
    TEST_ASSERT_TRUE(c.mejorasActivas());
    c.ejecutar({TipoAccion::EvadirBordeIzq},m,1000);
    TEST_ASSERT_EQUAL_INT(0,c.rutinaActual());
    TEST_ASSERT_TRUE(c.esModo4());
    TEST_ASSERT_EQUAL_INT((int)TipoAccion::EsquivarIzq,(int)estrategia.decidir(frontal()).tipo);
}
void test_reinicio_olvida_esquiva_y_vuelve_a_proteger_arranque() {
    forzarAccion(8); ControlMovimiento c; EstrategiaReglas clasica;
    EstrategiaCompetencia estrategia(clasica,c);
    c.iniciarRutina(4,1); estrategia.decidir(frontal());
    c.reiniciar(); c.iniciarRutina(1,1);
    TEST_ASSERT_FALSE(c.mejorasActivas());
    TEST_ASSERT_EQUAL_INT((int)TipoAccion::AtaqueFrontal,(int)estrategia.decidir(frontal()).tipo);
    forzarAccion(9); c.reiniciar(); c.iniciarRutina(4,1); g_millis=1001;
    TEST_ASSERT_EQUAL_INT((int)TipoAccion::EsquivarDer,(int)estrategia.decidir(frontal()).tipo);
}
void test_red_no_puede_ignorar_borde() {
    forzarAccion(9); EstrategiaRed e; auto l=frontal();l.lineaIzq=l.lineaDer=true;
    TEST_ASSERT_EQUAL_INT((int)TipoAccion::EvadirBordeAmbos,(int)e.decidir(l).tipo);
}
void test_esquiva_tiene_tiempo_y_rearme() {
    forzarAccion(8); EstrategiaRed e;
    TEST_ASSERT_TRUE(esEsquiva(e.decidir(frontal()).tipo));
    forzarAccion(0);g_millis+=1;
    TEST_ASSERT_EQUAL_INT((int)TipoAccion::EsquivarIzq,(int)e.decidir(frontal()).tipo);
    forzarAccion(8);g_millis=1000+TiempoMinimoEsquivaRed;
    TEST_ASSERT_FALSE(esEsquiva(e.decidir(frontal()).tipo));
    g_millis=1000+TiempoRearmeEsquivaRed;
    TEST_ASSERT_TRUE(esEsquiva(e.decidir(frontal()).tipo));
}
void test_correccion_sigue_limitada_por_borde_y_pwm() {
    std::memset(redcontrol::Parametros,0,redcontrol::numParametros());
    const int h=(redcontrol::numParametros()-2)/(redcontrol::NumEntradasRed+3);
    redcontrol::Parametros[0]=127;
    redcontrol::Parametros[h*(redcontrol::NumEntradasRed+1)]=127;
    ControlRedModo4 red;MandoMotores mando(0);Motor m;
    DecisionMovimiento d{TipoAccion::AtaqueFrontal,true,false,false,1000};
    red.ejecutar(d,mando,m,1000,255,false,255);
    TEST_ASSERT_INT_WITHIN(255,0,m.izq);TEST_ASSERT_INT_WITHIN(255,0,m.der);
    mando.limites.cercaBorde=true;
    red.ejecutar(d,mando,m,1010,255,false,255);
    TEST_ASSERT_TRUE(m.izq<=VelocidadCercaBorde && m.der<=VelocidadCercaBorde);
}
void test_segundo_sensor_amplia_solo_recuperacion_mejorada() {
    for(int mejorada=0;mejorada<=1;++mejorada) {
        ManiobraEvasion e;MandoMotores mando(0);Motor m;BusquedaRival b;
        e.configurarModo4(mejorada);e.iniciar(1,90,100,1000,true);
        e.iniciar(1,200,100,1010,false);
        e.continuar({TipoAccion::Busqueda},mando,m,1010,b);
        e.continuar({TipoAccion::Busqueda},mando,m,1110,b);
        TEST_ASSERT_EQUAL_INT(mejorada?-VelocidadRetroceso:0,m.izq);
    }
}
void test_pid_compacto_equivale_a_la_formula_anterior() {
    ControlPID pid;bool activo=false;int anterior=0,derivada=0;unsigned long ultimo=0,ahora=1000;
    unsigned long rng=991;
    for(int i=0;i<3000;++i) {
        rng=(rng*1664525UL+1013904223UL)&0xffffffffUL;
        int error=(int)(rng%2001)-1000;ahora+=i%19==0?70:i%3==0?3:10;
        long p=(long)error*92/64;
        if(!activo || ahora-ultimo>PausaReinicioPID) {activo=true;anterior=error;ultimo=ahora;derivada=0;}
        else if(ahora-ultimo>=PeriodoPID) {long cambio=(long)(error-anterior)*1024/1000;derivada+=(cambio-derivada)*128/256;anterior=error;ultimo=ahora;}
        long u=p+derivada;u=u < -1024?-1024:u>1024?1024:u;
        TEST_ASSERT_EQUAL_INT((int)u,pid.correccion(error,ahora));
    }
}
void test_primera_lectura_frontal_no_inventa_contacto_pasado() {
    std::memset(redlogica::Parametros,0,redlogica::numParametros());
    const int e=redlogica::NumEntradasRed, n=redlogica::numParametros();
    const int h=(n-redlogica::NumAcciones)/(e+1+redlogica::NumAcciones);
    redlogica::Parametros[8]=16;
    redlogica::Parametros[h*(e+1)+h]=1;
    redlogica::Parametros[h*(e+1)+8*(h+1)]=1;
    EstrategiaRed red;g_millis=5000;
    TEST_ASSERT_FALSE(esEsquiva(red.decidir(frontal()).tipo));
    g_millis=5000+ContactoFrontalMs-1;
    TEST_ASSERT_FALSE(esEsquiva(red.decidir(frontal()).tipo));
    g_millis=5000+ContactoFrontalMs;
    TEST_ASSERT_TRUE(esEsquiva(red.decidir(frontal()).tipo));
}
void test_control_modo3_no_cambia_al_entrenar_otros_modos() {
    ControlRedModo4 anterior,nuevo;MandoMotores mandoA(0),mandoB(0);Motor a,b;
    DecisionMovimiento d{TipoAccion::AjusteIzq,true,false,false,-500};
    anterior.ejecutar(d,mandoA,a,1000,130,false,255,3);
    std::memset(redcontrol::Parametros,127,redcontrol::numParametros());
    nuevo.ejecutar(d,mandoB,b,1000,130,false,255,3);
    TEST_ASSERT_EQUAL_INT(a.izq,b.izq);TEST_ASSERT_EQUAL_INT(a.der,b.der);
    TEST_ASSERT_EQUAL_INT(130*PorcentajeAjuste/100,b.izq);
    TEST_ASSERT_EQUAL_INT(130,b.der);
}

int main(int,char**) {
    UNITY_BEGIN();
    RUN_TEST(test_arranques_protegidos_y_mejoras_solo_al_terminar);
    RUN_TEST(test_modo4_mejora_desde_inicio_y_no_se_pierde_al_cancelar_rutina);
    RUN_TEST(test_reinicio_olvida_esquiva_y_vuelve_a_proteger_arranque);
    RUN_TEST(test_red_no_puede_ignorar_borde);
    RUN_TEST(test_esquiva_tiene_tiempo_y_rearme);
    RUN_TEST(test_correccion_sigue_limitada_por_borde_y_pwm);
    RUN_TEST(test_segundo_sensor_amplia_solo_recuperacion_mejorada);
    RUN_TEST(test_pid_compacto_equivale_a_la_formula_anterior);
    RUN_TEST(test_primera_lectura_frontal_no_inventa_contacto_pasado);
    RUN_TEST(test_control_modo3_no_cambia_al_entrenar_otros_modos);
    return UNITY_END();
}
