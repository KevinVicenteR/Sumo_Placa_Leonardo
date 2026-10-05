#include <unity.h>
#include "ModuloArranque.H"

void test_parado_hasta_recibir_start(void) {
    ModuloArranque m(5);
    TEST_ASSERT_FALSE(m.enMarcha(false, 0));
    TEST_ASSERT_FALSE(m.enMarcha(false, 100));
}

void test_start_tras_el_antirrebote(void) {
    ModuloArranque m(5);
    m.enMarcha(false, 100);
    TEST_ASSERT_FALSE(m.enMarcha(true, 101));
    TEST_ASSERT_FALSE(m.enMarcha(true, 103));
    TEST_ASSERT_TRUE(m.enMarcha(true, 106));
}

void test_un_pico_suelto_no_arranca(void) {
    ModuloArranque m(5);
    m.enMarcha(false, 100);
    TEST_ASSERT_FALSE(m.enMarcha(true, 101));
    TEST_ASSERT_FALSE(m.enMarcha(false, 102));
    TEST_ASSERT_FALSE(m.enMarcha(false, 110));
}

void test_stop_tras_el_antirrebote(void) {
    ModuloArranque m(5);
    m.enMarcha(false, 100);
    m.enMarcha(true, 101);
    TEST_ASSERT_TRUE(m.enMarcha(true, 106));
    TEST_ASSERT_TRUE(m.enMarcha(false, 107));
    TEST_ASSERT_FALSE(m.enMarcha(false, 112));
}

int main(int, char**) {
    UNITY_BEGIN();
    RUN_TEST(test_parado_hasta_recibir_start);
    RUN_TEST(test_start_tras_el_antirrebote);
    RUN_TEST(test_un_pico_suelto_no_arranca);
    RUN_TEST(test_stop_tras_el_antirrebote);
    return UNITY_END();
}
