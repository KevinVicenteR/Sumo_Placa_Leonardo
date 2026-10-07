#include <unity.h>
#include "hardware/CajaNegra.h"

static LecturasSensores frontal(bool ve) {
    LecturasSensores l{};
    l.frontal = ve;
    return l;
}

void test_guarda_y_lee_un_combate(void) {
    CajaNegra c;
    c.borrar();
    c.empezar(2, 900, 880, 1000);
    c.observar(frontal(true), 0, 1000);
    c.observar(frontal(true), 1, 3500);   // frontal 2,5 s seguidos
    c.observar(frontal(false), 3, 4000);
    c.anotarInterrupcion();
    c.guardar();

    RegistroCombate r;
    TEST_ASSERT_TRUE(c.leer(0, r));
    TEST_ASSERT_EQUAL_UINT16(0, r.numero);
    TEST_ASSERT_EQUAL_UINT8(2, r.dip);
    TEST_ASSERT_EQUAL_UINT16(900, r.negroIzq);
    TEST_ASSERT_EQUAL_UINT16(880, r.negroDer);
    TEST_ASSERT_EQUAL_UINT16(30, r.duracionDecimas);
    TEST_ASSERT_EQUAL_UINT16(3, r.evasiones);
    TEST_ASSERT_EQUAL_UINT8(1, r.interrupciones);
    TEST_ASSERT_EQUAL_UINT8(25, r.maxEnemigo[2]);
    TEST_ASSERT_EQUAL_UINT8(0, r.maxEnemigo[0]);
    TEST_ASSERT_FALSE(c.leer(1, r));
}

void test_conserva_solo_los_ultimos_combates(void) {
    CajaNegra c;
    c.borrar();
    for (int i = 0; i < CajaNegra::NumRegistros + 3; i++) {
        c.empezar(i & 3, 900, 900, 0);
        c.observar(frontal(false), i, 100);
        c.guardar();
    }
    RegistroCombate r;
    TEST_ASSERT_TRUE(c.leer(0, r));
    TEST_ASSERT_EQUAL_UINT16(CajaNegra::NumRegistros + 2, r.numero);   // el más reciente
    TEST_ASSERT_TRUE(c.leer(CajaNegra::NumRegistros - 1, r));
    TEST_ASSERT_EQUAL_UINT16(3, r.numero);                               // el más antiguo que queda
    TEST_ASSERT_FALSE(c.leer(CajaNegra::NumRegistros, r));
}

void test_parar_y_seguir_el_mismo_combate_no_duplica(void) {
    CajaNegra c;
    c.borrar();
    c.empezar(0, 900, 900, 0);
    c.observar(frontal(false), 1, 1000);
    c.guardar();                          // STOP corto
    c.anotarInterrupcion();               // vuelve RUN: mismo combate
    c.observar(frontal(false), 2, 2000);
    c.guardar();                          // STOP final
    RegistroCombate r;
    TEST_ASSERT_TRUE(c.leer(0, r));
    TEST_ASSERT_EQUAL_UINT16(0, r.numero);
    TEST_ASSERT_EQUAL_UINT16(2, r.evasiones);
    TEST_ASSERT_FALSE(c.leer(1, r));
}

int main(int, char**) {
    UNITY_BEGIN();
    RUN_TEST(test_guarda_y_lee_un_combate);
    RUN_TEST(test_conserva_solo_los_ultimos_combates);
    RUN_TEST(test_parar_y_seguir_el_mismo_combate_no_duplica);
    return UNITY_END();
}
