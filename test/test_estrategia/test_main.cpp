#include <unity.h>
#include "estrategia/EstrategiaReglas.h"
#include "estrategia/PoliticaAprendida.h"
#include "estrategia/TablaPolitica.h"
#include "estrategia/EstrategiaPesos.h"
#include "estrategia/RedPesos.h"
#include "estrategia/RedLogica.h"
#include "estrategia/EstrategiaRed.h"
#include <string.h>
#include "Parametros.h"

static LecturasSensores L(
    bool lineaIzq,
    bool lineaDer,
    bool latIzq,
    bool c45Izq,
    bool frontal,
    bool c45Der,
    bool latDer
) {
    LecturasSensores s{};
    s.lineaIzq = lineaIzq;
    s.lineaDer = lineaDer;
    s.latIzq = latIzq;
    s.c45Izq = c45Izq;
    s.frontal = frontal;
    s.c45Der = c45Der;
    s.latDer = latDer;
    return s;
}

void test_prioriza_borde_sobre_todo(void) {
    EstrategiaReglas e;
    DecisionMovimiento d = e.decidir(L(true, false, true, true, true, true, true));
    TEST_ASSERT_EQUAL_INT((int)TipoAccion::EvadirBordeIzq, (int)d.tipo);
    TEST_ASSERT_FALSE(d.ataqueDirecto);
}

void test_frontal_sobre_45_y_laterales(void) {
    EstrategiaReglas e;
    DecisionMovimiento d = e.decidir(L(false, false, true, true, true, true, true));
    TEST_ASSERT_EQUAL_INT((int)TipoAccion::AtaqueFrontal, (int)d.tipo);
    TEST_ASSERT_TRUE(d.ataqueDirecto);
}

void test_frontal_y_un_45_ajusta_hacia_ese_lado(void) {
    EstrategiaReglas e;
    DecisionMovimiento d = e.decidir(L(false, false, false, true, true, false, false));
    TEST_ASSERT_EQUAL_INT((int)TipoAccion::AjusteIzq, (int)d.tipo);
    TEST_ASSERT_TRUE(d.enemigoFrente);
    d = e.decidir(L(false, false, false, false, true, true, false));
    TEST_ASSERT_EQUAL_INT((int)TipoAccion::AjusteDer, (int)d.tipo);
}

void test_borde_informa_si_ve_al_enemigo_de_frente(void) {
    EstrategiaReglas e;
    TEST_ASSERT_TRUE(e.decidir(L(true, false, false, false, true, false, false)).enemigoFrente);
    TEST_ASSERT_FALSE(e.decidir(L(true, false, false, true, false, false, false)).enemigoFrente);
}

void test_cerca_del_borde_se_informa_en_la_decision(void) {
    EstrategiaReglas e;
    LecturasSensores s = L(false, false, false, false, true, false, false);
    s.cercaBorde = true;
    TEST_ASSERT_TRUE(e.decidir(s).cercaBorde);
    TEST_ASSERT_FALSE(e.decidir(L(false, false, false, false, true, false, false)).cercaBorde);
}

// La política en tabla, sin entrenar, decide igual que la estrategia escrita a mano
void test_politica_manual_coincide_con_la_estrategia(void) {
    EstrategiaReglas e;
    for (int bits = 0; bits < 32; bits++) {
        LecturasSensores s = L(false, false, bits & 8, bits & 2, bits & 1, bits & 4, bits & 16);
        const int estado = politica::codificarEstado(s, 0);
        TEST_ASSERT_EQUAL_INT((int)e.decidir(s).tipo, (int)politica::Acciones[politica::accionManual(estado)]);
    }
}

void test_tabla_de_politica_con_acciones_validas(void) {
    for (int estado = 0; estado < politica::NumEstados; estado++) {
        TEST_ASSERT_TRUE(TablaPolitica[estado] < politica::NumAcciones);
    }
}

void test_45_izq_sobre_45_der_y_laterales(void) {
    EstrategiaReglas e;
    DecisionMovimiento d = e.decidir(L(false, false, true, true, false, true, true));
    TEST_ASSERT_EQUAL_INT((int)TipoAccion::CorregirIzq, (int)d.tipo);
}

void test_borde_der_y_ambos(void) {
    EstrategiaReglas e;
    TEST_ASSERT_EQUAL_INT((int)TipoAccion::EvadirBordeDer, (int)e.decidir(L(false, true, false, false, true, false, false)).tipo);
    TEST_ASSERT_EQUAL_INT((int)TipoAccion::EvadirBordeAmbos, (int)e.decidir(L(true, true, false, false, true, false, false)).tipo);
}

void test_45_der_sobre_laterales(void) {
    EstrategiaReglas e;
    DecisionMovimiento d = e.decidir(L(false, false, true, false, false, true, true));
    TEST_ASSERT_EQUAL_INT((int)TipoAccion::CorregirDer, (int)d.tipo);
}

void test_laterales(void) {
    EstrategiaReglas e;
    TEST_ASSERT_EQUAL_INT((int)TipoAccion::DefensaIzq, (int)e.decidir(L(false, false, true, false, false, false, true)).tipo);
    TEST_ASSERT_EQUAL_INT((int)TipoAccion::DefensaDer, (int)e.decidir(L(false, false, false, false, false, false, true)).tipo);
}

void test_busqueda_si_no_hay_deteccion(void) {
    EstrategiaReglas e;
    DecisionMovimiento d = e.decidir(L(false, false, false, false, false, false, false));
    TEST_ASSERT_EQUAL_INT((int)TipoAccion::Busqueda, (int)d.tipo);
}

// La red de pesos de partida decide igual que las reglas, con las tres activaciones
void test_red_pesos_coincide_con_la_estrategia(void) {
    EstrategiaReglas e;
    const int funciones[] = {redpesos::Sigmoide, redpesos::Relu, redpesos::Tanh};
    for (int f : funciones) {
        for (int bits = 0; bits < 64; bits++) {
            LecturasSensores s = L(false, false, bits & 8, bits & 2, bits & 1, bits & 4, bits & 16);
            int8_t x[redpesos::NumEntradas];
            redpesos::entradas(s, bits & 32, x);
            const int n = redpesos::elegirNeurona(x, f, 0.5f);
            TEST_ASSERT_EQUAL_INT((int)e.decidir(s).tipo, (int)redpesos::Acciones[n]);
        }
    }
}

void test_estrategia_pesos_respeta_borde_y_decide(void) {
    EstrategiaPesos e;
    TEST_ASSERT_EQUAL_INT((int)TipoAccion::EvadirBordeIzq,
                          (int)e.decidir(L(true, false, false, false, true, false, false)).tipo);
    TEST_ASSERT_EQUAL_INT((int)TipoAccion::CorregirDer,
                          (int)e.decidir(L(false, false, false, false, false, true, false)).tipo);
    // La neurona ganadora es la más activada
    for (int n = 0; n < redpesos::NumNeuronas; n++) {
        TEST_ASSERT_TRUE(e.activacion(n) <= e.activacion(4));
    }
}

void test_funciones_de_activacion(void) {
    TEST_ASSERT_EQUAL_FLOAT(0.5f, redpesos::activar(0, redpesos::Sigmoide));
    TEST_ASSERT_TRUE(redpesos::activar(5, redpesos::Sigmoide) > 0.8f);
    TEST_ASSERT_TRUE(redpesos::activar(-5, redpesos::Sigmoide) < 0.2f);
    TEST_ASSERT_EQUAL_FLOAT(0.0f, redpesos::activar(-3, redpesos::Relu));
    TEST_ASSERT_EQUAL_FLOAT(3.0f, redpesos::activar(3, redpesos::Relu));
    TEST_ASSERT_TRUE(redpesos::activar(-5, redpesos::Tanh) < -0.8f);
}

// Si ninguna neurona llega al umbral, el robot busca
void test_umbral_de_activacion_manda_a_buscar(void) {
    int8_t x[redpesos::NumEntradas];
    redpesos::entradas(L(false, false, false, false, true, false, false), 0, x);
    TEST_ASSERT_EQUAL_INT(0, redpesos::elegirNeurona(x, redpesos::Sigmoide, 0.5f));
    TEST_ASSERT_EQUAL_INT(redpesos::NeuronaBusqueda, redpesos::elegirNeurona(x, redpesos::Sigmoide, 0.99f));
}

// Error del PID: media de las posiciones de los sensores delanteros que ven al rival
void test_error_rival_para_el_pid(void) {
    EstrategiaReglas e;
    TEST_ASSERT_EQUAL_INT(0, e.decidir(L(false, false, false, false, true, false, false)).errorRival);
    TEST_ASSERT_EQUAL_INT(-PosicionSensor45PID, e.decidir(L(false, false, false, true, false, false, false)).errorRival);
    TEST_ASSERT_EQUAL_INT(PosicionSensor45PID / 2, e.decidir(L(false, false, false, false, true, true, false)).errorRival);
    TEST_ASSERT_EQUAL_INT(0, e.decidir(L(false, false, false, true, true, true, false)).errorRival);
}

// Red de lógica con pesos puestos a mano (se restauran al acabar)
static int8_t copiaRed[1024];
static void guardarRed() { memcpy(copiaRed, redlogica::Parametros, redlogica::numParametros()); }
static void restaurarRed() { memcpy(redlogica::Parametros, copiaRed, redlogica::numParametros()); }

void test_red_logica_elige_la_salida_mayor(void) {
    guardarRed();
    memset(redlogica::Parametros, 0, redlogica::numParametros());
    // Primera neurona oculta = 16 × frontal; la salida 3 la mira con peso 1
    const int e = redlogica::NumEntradasRed;
    const int ocultas = (redlogica::numParametros() - redlogica::NumAcciones) / (e + 1 + redlogica::NumAcciones);
    redlogica::Parametros[0] = 16;
    redlogica::Parametros[ocultas * (e + 1) + 3 * (ocultas + 1)] = 1;
    int8_t x[redlogica::NumEntradasRed] = {1};
    long sumas[redlogica::NumAcciones];
    TEST_ASSERT_EQUAL_INT(3, redlogica::elegir(x, 0, sumas));
    TEST_ASSERT_EQUAL_INT(16, sumas[3]);
    // Sin entradas todas valen 0: gana la primera si llega a la suma mínima
    x[0] = 0;
    TEST_ASSERT_EQUAL_INT(0, redlogica::elegir(x, 0));
    TEST_ASSERT_EQUAL_INT(redlogica::SalidaBusqueda, redlogica::elegir(x, 1));
    restaurarRed();
}

// La red puede esquivar con el rival centrado, pero nunca ignora la línea blanca.
void test_estrategia_red_puede_esquivar_y_respeta_borde(void) {
    guardarRed();
    memset(redlogica::Parametros, 0, redlogica::numParametros());
    const int e = redlogica::NumEntradasRed;
    const int n = redlogica::numParametros();
    const int h = (n - redlogica::NumAcciones) / (e + 1 + redlogica::NumAcciones);
    redlogica::Parametros[h * (e + 1) + 8 * (h + 1) + h] = 64;
    EstrategiaRed estrategia;
    const auto borde = estrategia.decidir(L(true, false, false, true, true, true, false));
    const auto centrado = estrategia.decidir(L(false, false, false, true, true, true, false));
    restaurarRed();
    TEST_ASSERT_EQUAL_INT((int)TipoAccion::EvadirBordeIzq, (int)borde.tipo);
    TEST_ASSERT_EQUAL_INT((int)TipoAccion::EsquivarIzq, (int)centrado.tipo);
    TEST_ASSERT_FALSE(centrado.ataqueDirecto);
}

extern unsigned long g_millis;
void test_esquiva_tiene_duracion_y_ventana_de_ataque(void) {
    guardarRed();
    memset(redlogica::Parametros, 0, redlogica::numParametros());
    const int e = redlogica::NumEntradasRed;
    const int n = redlogica::numParametros();
    const int h = (n - redlogica::NumAcciones) / (e + 1 + redlogica::NumAcciones);
    const int salida = h * (e + 1);
    redlogica::Parametros[salida + 8 * (h + 1) + h] = 64;
    EstrategiaRed estrategia;
    const auto rival = L(false, false, false, true, true, true, false);
    g_millis = 1000;
    const auto inicio = estrategia.decidir(rival);
    // La red cambia de opinión un ciclo después; el arco no debe cortarse.
    redlogica::Parametros[salida + h] = 100;
    g_millis = 1001;
    const auto sostenida = estrategia.decidir(rival);
    redlogica::Parametros[salida + h] = 0;
    g_millis = 1000 + TiempoMinimoEsquivaRed;
    const auto ataque = estrategia.decidir(rival);
    g_millis = 1000 + TiempoRearmeEsquivaRed;
    const auto rearmada = estrategia.decidir(rival);
    restaurarRed();
    g_millis = 0;
    TEST_ASSERT_EQUAL_INT((int)TipoAccion::EsquivarIzq, (int)inicio.tipo);
    TEST_ASSERT_EQUAL_INT((int)inicio.tipo, (int)sostenida.tipo);
    TEST_ASSERT_FALSE(esEsquiva(ataque.tipo));
    TEST_ASSERT_EQUAL_INT((int)inicio.tipo, (int)rearmada.tipo);
}


int main(int, char**) {
    UNITY_BEGIN();
    RUN_TEST(test_prioriza_borde_sobre_todo);
    RUN_TEST(test_frontal_sobre_45_y_laterales);
    RUN_TEST(test_frontal_y_un_45_ajusta_hacia_ese_lado);
    RUN_TEST(test_borde_informa_si_ve_al_enemigo_de_frente);
    RUN_TEST(test_cerca_del_borde_se_informa_en_la_decision);
    RUN_TEST(test_politica_manual_coincide_con_la_estrategia);
    RUN_TEST(test_tabla_de_politica_con_acciones_validas);
    RUN_TEST(test_45_izq_sobre_45_der_y_laterales);
    RUN_TEST(test_borde_der_y_ambos);
    RUN_TEST(test_45_der_sobre_laterales);
    RUN_TEST(test_laterales);
    RUN_TEST(test_busqueda_si_no_hay_deteccion);
    RUN_TEST(test_red_pesos_coincide_con_la_estrategia);
    RUN_TEST(test_estrategia_pesos_respeta_borde_y_decide);
    RUN_TEST(test_funciones_de_activacion);
    RUN_TEST(test_umbral_de_activacion_manda_a_buscar);
    RUN_TEST(test_error_rival_para_el_pid);
    RUN_TEST(test_red_logica_elige_la_salida_mayor);
    RUN_TEST(test_estrategia_red_puede_esquivar_y_respeta_borde);
    RUN_TEST(test_esquiva_tiene_duracion_y_ventana_de_ataque);
    return UNITY_END();
}
