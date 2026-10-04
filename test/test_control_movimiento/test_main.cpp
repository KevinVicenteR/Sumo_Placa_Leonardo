#include <unity.h>
#include "ControlMovimiento.H"
#include "EstrategiaCombate.H"
#include "Pines.H"
#include "Arduino.h"

class MotorMock : public IMotor {
public:
    int izq = 0, der = 0;
    void avanzar(int) override {}
    void retroceder(int) override {}
    void detener() override {}
    void girar(int) override {}
    void curva(int) override {}
    void mover(int i, int d) override { izq = i; der = d; }
};
void setUp() { testMillis() = 0; g_delayCallCount = 0; }
void tearDown() {}

void test_borde_tiene_prioridad_sobre_todos_los_enemigos() {
    LecturasSensores l{};
    l.frontal = l.c45Izq = l.c45Der = l.latIzq = l.latDer = true;
    EstrategiaCombate e;
    l.lineaIzq = true;
    TEST_ASSERT_EQUAL_INT((int)TipoAccion::EvadirBordeIzq, (int)e.decidir(l).tipo);
    l.lineaDer = true;
    TEST_ASSERT_EQUAL_INT((int)TipoAccion::EvadirBordeAmbos, (int)e.decidir(l).tipo);
    l.lineaIzq = false;
    TEST_ASSERT_EQUAL_INT((int)TipoAccion::EvadirBordeDer, (int)e.decidir(l).tipo);
}
void test_enemigo_no_cancela_retirada_y_giro() {
    ControlMovimiento c; MotorMock m;
    c.ejecutar({TipoAccion::EvadirBordeAmbos}, m);
    TEST_ASSERT_EQUAL_INT(-VelocidadRetroceso, m.izq);
    TEST_ASSERT_EQUAL_INT(-VelocidadRetroceso, m.der);
    testMillis() = 100;
    c.ejecutar({TipoAccion::AtaqueFrontal}, m);
    TEST_ASSERT_TRUE(m.izq < 0 && m.der < 0);
    testMillis() = 220;
    c.ejecutar({TipoAccion::AtaqueFrontal}, m);
    testMillis() = 221;
    c.ejecutar({TipoAccion::AtaqueFrontal}, m);
    TEST_ASSERT_EQUAL_INT(VelocidadGiro, m.izq);
    TEST_ASSERT_EQUAL_INT(-VelocidadGiro, m.der);
    testMillis() = 400;
    c.ejecutar({TipoAccion::AtaqueFrontal}, m);
    testMillis() = 401;
    c.ejecutar({TipoAccion::AtaqueFrontal}, m);
    TEST_ASSERT_EQUAL_INT(0, m.izq);
    testMillis() = 500;
    c.ejecutar({TipoAccion::AtaqueFrontal}, m);
    testMillis() = 501;
    c.ejecutar({TipoAccion::AtaqueFrontal}, m);
    TEST_ASSERT_EQUAL_INT(VelocidadAtaqueFrontal, m.izq);
    TEST_ASSERT_EQUAL_INT(0, g_delayCallCount);
}
void test_linea_persistente_impide_giro() {
    ControlMovimiento c; MotorMock m;
    c.ejecutar({TipoAccion::EvadirBordeAmbos}, m);
    testMillis() = 1000;
    c.ejecutar({TipoAccion::EvadirBordeAmbos}, m);
    testMillis() = 1079;
    c.ejecutar({TipoAccion::AtaqueFrontal}, m);
    TEST_ASSERT_TRUE(m.izq < 0 && m.der < 0);
    testMillis() = 1080;
    c.ejecutar({TipoAccion::Busqueda}, m);
    testMillis() = 1081;
    c.ejecutar({TipoAccion::Busqueda}, m);
    TEST_ASSERT_TRUE(m.izq * m.der < 0);
}
void test_borde_durante_giro_reinicia_retroceso() {
    ControlMovimiento c; MotorMock m;
    c.ejecutar({TipoAccion::EvadirBordeAmbos}, m);
    testMillis() = 220;
    c.ejecutar({TipoAccion::Busqueda}, m);
    testMillis() = 230;
    c.ejecutar({TipoAccion::EvadirBordeAmbos}, m);
    TEST_ASSERT_TRUE(m.izq < 0 && m.der < 0);
    testMillis() = 440;
    c.ejecutar({TipoAccion::AtaqueFrontal}, m);
    TEST_ASSERT_TRUE(m.izq < 0 && m.der < 0);
}
void test_un_sensor_solo_gira_alejandose_del_borde() {
    ControlMovimiento c; MotorMock m;
    c.ejecutar({TipoAccion::EvadirBordeIzq}, m);
    // Nariz hacia la derecha: rueda derecha retrocede más que la izquierda.
    TEST_ASSERT_TRUE(m.der < m.izq && m.izq <= 0);
    testMillis() = 89;
    c.ejecutar({TipoAccion::AtaqueFrontal}, m);
    TEST_ASSERT_TRUE(m.der < m.izq);
    testMillis() = 90;
    c.ejecutar({TipoAccion::AtaqueFrontal}, m);
    testMillis() = 91;
    c.ejecutar({TipoAccion::AtaqueFrontal}, m);
    // Sin retroceso largo ni pausa: vuelve a atacar enseguida.
    TEST_ASSERT_EQUAL_INT(VelocidadAtaqueFrontal, m.izq);

    c.ejecutar({TipoAccion::EvadirBordeDer}, m);
    TEST_ASSERT_TRUE(m.izq < m.der && m.der <= 0);
}
void test_esquina_durante_pivote_obliga_a_retroceder() {
    ControlMovimiento c; MotorMock m;
    c.ejecutar({TipoAccion::EvadirBordeIzq}, m);
    testMillis() = 10;
    c.ejecutar({TipoAccion::EvadirBordeDer}, m);
    TEST_ASSERT_EQUAL_INT(-VelocidadRetroceso, m.izq);
    TEST_ASSERT_EQUAL_INT(-VelocidadRetroceso, m.der);
}
void test_busqueda_no_avanza_y_ataque_frontal_centrado() {
    ControlMovimiento c; MotorMock m; EstrategiaCombate e;
    c.ejecutar({TipoAccion::Busqueda}, m);
    TEST_ASSERT_EQUAL_INT(0, m.izq + m.der);
    LecturasSensores l{}; l.frontal = true;
    DecisionMovimiento d = e.decidir(l);
    TEST_ASSERT_EQUAL_INT(0, d.error);
    c.ejecutar(d, m);
    TEST_ASSERT_EQUAL_INT(VelocidadAtaqueFrontal, m.izq);
    TEST_ASSERT_EQUAL_INT(m.izq, m.der);
}
int main(int, char**) {
    UNITY_BEGIN();
    RUN_TEST(test_borde_tiene_prioridad_sobre_todos_los_enemigos);
    RUN_TEST(test_enemigo_no_cancela_retirada_y_giro);
    RUN_TEST(test_linea_persistente_impide_giro);
    RUN_TEST(test_borde_durante_giro_reinicia_retroceso);
    RUN_TEST(test_un_sensor_solo_gira_alejandose_del_borde);
    RUN_TEST(test_esquina_durante_pivote_obliga_a_retroceder);
    RUN_TEST(test_busqueda_no_avanza_y_ataque_frontal_centrado);
    return UNITY_END();
}
