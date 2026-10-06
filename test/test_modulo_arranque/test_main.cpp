#include <unity.h>
#include "ModuloArranque.H"

void test_parado_hasta_recibir_start(void) {
    ModuloArranque m(5, 150);
    TEST_ASSERT_FALSE(m.enMarcha(false, 0));
    TEST_ASSERT_FALSE(m.enMarcha(false, 100));
}

void test_start_tras_el_antirrebote(void) {
    ModuloArranque m(5, 150);
    m.enMarcha(false, 100);
    TEST_ASSERT_FALSE(m.enMarcha(true, 101));
    TEST_ASSERT_FALSE(m.enMarcha(true, 103));  // 2 ms: todavía no
    TEST_ASSERT_TRUE(m.enMarcha(true, 106));   // 5 ms seguidos: RUN
}

void test_un_pico_suelto_no_arranca(void) {
    ModuloArranque m(5, 150);
    m.enMarcha(false, 100);
    TEST_ASSERT_FALSE(m.enMarcha(true, 101));
    TEST_ASSERT_FALSE(m.enMarcha(false, 102));  // ruido de 1 ms
    TEST_ASSERT_FALSE(m.enMarcha(false, 110));
}

void test_caida_corta_no_para_el_combate(void) {
    ModuloArranque m(5, 150);
    m.enMarcha(false, 100);
    m.enMarcha(true, 101);
    TEST_ASSERT_TRUE(m.enMarcha(true, 106));
    TEST_ASSERT_TRUE(m.enMarcha(false, 110));   // ruido del rival: 100 ms en STOP
    TEST_ASSERT_TRUE(m.enMarcha(false, 210));
    TEST_ASSERT_TRUE(m.enMarcha(true, 211));    // la señal vuelve: sigue en marcha
    TEST_ASSERT_TRUE(m.enMarcha(false, 212));
    TEST_ASSERT_TRUE(m.enMarcha(false, 300));   // el filtro empezó de nuevo
}

void test_stop_tras_el_antirrebote(void) {
    ModuloArranque m(5, 150);
    m.enMarcha(false, 100);
    m.enMarcha(true, 101);
    TEST_ASSERT_TRUE(m.enMarcha(true, 106));
    TEST_ASSERT_TRUE(m.enMarcha(false, 107));
    TEST_ASSERT_TRUE(m.enMarcha(false, 200));
    TEST_ASSERT_FALSE(m.enMarcha(false, 257));  // 150 ms seguidos: STOP
}

int main(int, char**) {
    UNITY_BEGIN();
    RUN_TEST(test_parado_hasta_recibir_start);
    RUN_TEST(test_start_tras_el_antirrebote);
    RUN_TEST(test_un_pico_suelto_no_arranca);
    RUN_TEST(test_caida_corta_no_para_el_combate);
    RUN_TEST(test_stop_tras_el_antirrebote);
    return UNITY_END();
}
