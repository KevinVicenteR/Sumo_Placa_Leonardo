#include <unity.h>
#include "Percepcion.H"
#include "Pines.H"
#include "Arduino.h"

void test_blanco_persistente_no_se_recalibra_como_negro() {
    Percepcion p;
    g_analogValues[S_PISO_IZQ] = 800;
    g_analogValues[S_PISO_DER] = 800;
    p.calibrarPiso();
    g_analogValues[S_PISO_IZQ] = 200;
    for (g_millis = 0; g_millis < 2000; g_millis += 10) {
        TEST_ASSERT_TRUE(p.leer().lineaIzq);
        TEST_ASSERT_FALSE(p.leer().lineaDer);
    }
    g_analogValues[S_PISO_IZQ] = 800;
    TEST_ASSERT_FALSE(p.leer().lineaIzq);
}
void test_polaridad_inversa_e_histeresis() {
    Percepcion p;
    g_analogValues[S_PISO_IZQ] = 100;
    g_analogValues[S_PISO_DER] = 100;
    p.calibrarPiso();
    g_analogValues[S_PISO_IZQ] = 200;
    TEST_ASSERT_TRUE(p.leer().lineaIzq);
    g_analogValues[S_PISO_IZQ] = 150;
    TEST_ASSERT_TRUE(p.leer().lineaIzq);
    g_analogValues[S_PISO_IZQ] = 120;
    TEST_ASSERT_FALSE(p.leer().lineaIzq);
}
int main(int, char**) {
    UNITY_BEGIN();
    RUN_TEST(test_blanco_persistente_no_se_recalibra_como_negro);
    RUN_TEST(test_polaridad_inversa_e_histeresis);
    return UNITY_END();
}
