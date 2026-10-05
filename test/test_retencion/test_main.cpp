#include <unity.h>
#include "RetencionDeteccion.H"

void test_sin_deteccion_inicial(void) {
    RetencionDeteccion r(40);
    TEST_ASSERT_FALSE(r.actualizar(false, 0));
    TEST_ASSERT_FALSE(r.actualizar(false, 1000));
}

void test_mantiene_deteccion_durante_la_ventana(void) {
    RetencionDeteccion r(40);
    TEST_ASSERT_TRUE(r.actualizar(true, 1000));
    TEST_ASSERT_TRUE(r.actualizar(false, 1039));
    TEST_ASSERT_FALSE(r.actualizar(false, 1040));
    TEST_ASSERT_FALSE(r.actualizar(false, 1041));
}

void test_nueva_deteccion_reinicia_ventana(void) {
    RetencionDeteccion r(40);
    r.actualizar(true, 1000);
    r.actualizar(true, 1030);
    TEST_ASSERT_TRUE(r.actualizar(false, 1060));
    TEST_ASSERT_FALSE(r.actualizar(false, 1070));
}

void test_funciona_con_desborde_de_millis(void) {
    RetencionDeteccion r(40);
    const unsigned long casiDesborde = (unsigned long)-16;
    r.actualizar(true, casiDesborde);
    TEST_ASSERT_TRUE(r.actualizar(false, 16));
    TEST_ASSERT_FALSE(r.actualizar(false, 30));
}

void test_una_lectura_suelta_no_cuenta_si_hay_confirmacion(void) {
    RetencionDeteccion r(40, 5);
    TEST_ASSERT_FALSE(r.actualizar(true, 100));
    TEST_ASSERT_FALSE(r.actualizar(false, 102));
    TEST_ASSERT_FALSE(r.actualizar(true, 110));
    TEST_ASSERT_TRUE(r.actualizar(true, 115));
    TEST_ASSERT_TRUE(r.actualizar(false, 140));
    TEST_ASSERT_FALSE(r.actualizar(false, 160));
}

int main(int, char**) {
    UNITY_BEGIN();
    RUN_TEST(test_sin_deteccion_inicial);
    RUN_TEST(test_mantiene_deteccion_durante_la_ventana);
    RUN_TEST(test_nueva_deteccion_reinicia_ventana);
    RUN_TEST(test_funciona_con_desborde_de_millis);
    RUN_TEST(test_una_lectura_suelta_no_cuenta_si_hay_confirmacion);
    return UNITY_END();
}
