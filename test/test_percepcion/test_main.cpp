#include <unity.h>
#include "sensores/Percepcion.h"
#include "Pines.h"
#include "Arduino.h"

void test_piso_arranca_con_referencias_guardadas_sin_calibracion() {
    g_analogValues[S_PISO_IZQ] = 54;
    g_analogValues[S_PISO_DER] = 56;
    Percepcion p;
    TEST_ASSERT_EQUAL(994, p.negroIzquierdo());
    TEST_ASSERT_EQUAL(972, p.negroDerecho());
    TEST_ASSERT_EQUAL(149, p.margenIzquierdo());
    TEST_ASSERT_EQUAL(145, p.margenDerecho());
    LecturasSensores l = p.leer();
    TEST_ASSERT_TRUE(l.lineaIzq && l.lineaDer);
    g_analogValues[S_PISO_IZQ] = 994;
    g_analogValues[S_PISO_DER] = 972;
    l = p.leer();
    TEST_ASSERT_FALSE(l.lineaIzq || l.lineaDer);
}
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
void test_calibra_cada_enemigo_y_reinicia_retencion() {
    Percepcion p;
    const uint8_t pins[] = {S_LAT_IZQ, S_FRONT_IZQ, S_FRONT_CEN, S_FRONT_DER, S_LAT_DER};
    for (int i = 0; i < 5; ++i) g_digitalValues[pins[i]] = i % 2;
    p.calibrarSensoresEnemigo();
    LecturasSensores l = p.leer();
    TEST_ASSERT_FALSE(l.latIzq || l.c45Izq || l.frontal || l.c45Der || l.latDer);
    for (int i = 0; i < 5; ++i) g_digitalValues[pins[i]] = !(i % 2);
    l = p.leer();
    TEST_ASSERT_TRUE(l.latIzq && l.c45Izq && l.frontal && l.c45Der && l.latDer);
    p.calibrarSensoresEnemigo();
    l = p.leer();
    TEST_ASSERT_FALSE(l.latIzq || l.c45Izq || l.frontal || l.c45Der || l.latDer);
}
void test_competencia_detecta_enemigo_presente_al_encender_sin_calibrar() {
    const uint8_t pins[] = {S_LAT_IZQ, S_FRONT_IZQ, S_FRONT_CEN, S_FRONT_DER, S_LAT_DER};
    for (uint8_t pin : pins) g_digitalValues[pin] = HIGH;
    Percepcion p;
    const LecturasSensores l = p.leer();
    TEST_ASSERT_TRUE(l.latIzq && l.c45Izq && l.frontal && l.c45Der && l.latDer);
}
int main(int, char**) {
    UNITY_BEGIN();
    RUN_TEST(test_competencia_detecta_enemigo_presente_al_encender_sin_calibrar);
    RUN_TEST(test_piso_arranca_con_referencias_guardadas_sin_calibracion);
    RUN_TEST(test_calibra_cada_enemigo_y_reinicia_retencion);
    RUN_TEST(test_blanco_persistente_no_se_recalibra_como_negro);
    RUN_TEST(test_polaridad_inversa_e_histeresis);
    return UNITY_END();
}
