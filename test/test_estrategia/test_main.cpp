#include <unity.h>
#include "estrategia/EstrategiaReglas.h"
#include "estrategia/PoliticaAprendida.h"
#include "estrategia/TablaPolitica.h"

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
    return UNITY_END();
}
