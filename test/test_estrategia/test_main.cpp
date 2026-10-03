#include <unity.h>
#include "EstrategiaCombate.H"

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
    EstrategiaCombate e;
    DecisionMovimiento d = e.decidir(L(true, false, true, true, true, true, true));
    TEST_ASSERT_EQUAL_INT((int)TipoAccion::EvadirBordeIzq, (int)d.tipo);
}

void test_frontal_sobre_45_y_laterales(void) {
    EstrategiaCombate e;
    DecisionMovimiento d = e.decidir(L(false, false, true, true, true, true, true));
    TEST_ASSERT_EQUAL_INT((int)TipoAccion::AtaqueFrontal, (int)d.tipo);
}

void test_45_izq_sobre_45_der_y_laterales(void) {
    EstrategiaCombate e;
    DecisionMovimiento d = e.decidir(L(false, false, true, true, false, true, true));
    TEST_ASSERT_EQUAL_INT((int)TipoAccion::CorregirIzq, (int)d.tipo);
}

void test_borde_der_y_ambos(void) {
    EstrategiaCombate e;
    TEST_ASSERT_EQUAL_INT((int)TipoAccion::EvadirBordeDer, (int)e.decidir(L(false, true, false, false, true, false, false)).tipo);
    TEST_ASSERT_EQUAL_INT((int)TipoAccion::EvadirBordeAmbos, (int)e.decidir(L(true, true, false, false, true, false, false)).tipo);
}

void test_45_der_sobre_laterales(void) {
    EstrategiaCombate e;
    DecisionMovimiento d = e.decidir(L(false, false, true, false, false, true, true));
    TEST_ASSERT_EQUAL_INT((int)TipoAccion::CorregirDer, (int)d.tipo);
}

void test_laterales(void) {
    EstrategiaCombate e;
    TEST_ASSERT_EQUAL_INT((int)TipoAccion::DefensaIzq, (int)e.decidir(L(false, false, true, false, false, false, true)).tipo);
    TEST_ASSERT_EQUAL_INT((int)TipoAccion::DefensaDer, (int)e.decidir(L(false, false, false, false, false, false, true)).tipo);
}

void test_busqueda_si_no_hay_deteccion(void) {
    EstrategiaCombate e;
    DecisionMovimiento d = e.decidir(L(false, false, false, false, false, false, false));
    TEST_ASSERT_EQUAL_INT((int)TipoAccion::Busqueda, (int)d.tipo);
}

int main(int, char**) {
    UNITY_BEGIN();
    RUN_TEST(test_prioriza_borde_sobre_todo);
    RUN_TEST(test_frontal_sobre_45_y_laterales);
    RUN_TEST(test_45_izq_sobre_45_der_y_laterales);
    RUN_TEST(test_borde_der_y_ambos);
    RUN_TEST(test_45_der_sobre_laterales);
    RUN_TEST(test_laterales);
    RUN_TEST(test_busqueda_si_no_hay_deteccion);
    return UNITY_END();
}
