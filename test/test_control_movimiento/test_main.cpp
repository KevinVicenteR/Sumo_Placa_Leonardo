#include <unity.h>
#include "ControlMovimiento.H"
#include "Parametros.H"
#include "Arduino.h"

class MotorMock : public IMotor {
public:
    int calls = 0;
    int izq = 0;
    int der = 0;

    void detener() override { mover(0, 0); }

    void mover(int velIzq, int velDer) override {
        izq = velIzq;
        der = velDer;
        calls++;
    }
};

static void assertMovimiento(MotorMock& m, int izq, int der) {
    TEST_ASSERT_EQUAL_INT(izq, m.izq);
    TEST_ASSERT_EQUAL_INT(der, m.der);
}

static void assertAccionSimple(TipoAccion accion, int izq, int der) {
    ControlMovimiento c;
    MotorMock m;
    g_delayCallCount = 0;

    c.ejecutar({accion}, m, 0);

    TEST_ASSERT_EQUAL_INT(1, m.calls);
    assertMovimiento(m, izq, der);
    TEST_ASSERT_EQUAL_INT(0, g_delayCallCount);
}

void test_acciones_simples_velocidades_correctas(void) {
    assertAccionSimple(TipoAccion::AtaqueFrontal, VelocidadMaxima, VelocidadMaxima);
    assertAccionSimple(TipoAccion::CorregirIzq, VelocidadCurva, VelocidadMaxima);
    assertAccionSimple(TipoAccion::CorregirDer, VelocidadMaxima, VelocidadCurva);
    assertAccionSimple(TipoAccion::DefensaIzq, -VelocidadPivoteLateral, VelocidadMaxima);
    assertAccionSimple(TipoAccion::DefensaDer, VelocidadMaxima, -VelocidadPivoteLateral);
    assertAccionSimple(TipoAccion::Busqueda, VelocidadAvance, VelocidadBusquedaDer);
}

void test_evadir_borde_izq_retrocede_y_gira_derecha_sin_bloquear(void) {
    ControlMovimiento c;
    MotorMock m;
    g_delayCallCount = 0;

    c.ejecutar({TipoAccion::EvadirBordeIzq}, m, 1000);
    assertMovimiento(m, -VelocidadRetroceso, -VelocidadRetroceso);

    // La línea ya no se ve, pero la maniobra continúa
    c.ejecutar({TipoAccion::Busqueda}, m, 1000 + TiempoRetroceso - 1);
    assertMovimiento(m, -VelocidadRetroceso, -VelocidadRetroceso);

    c.ejecutar({TipoAccion::Busqueda}, m, 1000 + TiempoRetroceso);
    assertMovimiento(m, VelocidadMaxima, -VelocidadMaxima);

    c.ejecutar({TipoAccion::Busqueda}, m, 1000 + TiempoRetroceso + TiempoGiroEvasion - 1);
    assertMovimiento(m, VelocidadMaxima, -VelocidadMaxima);

    c.ejecutar({TipoAccion::Busqueda}, m, 1000 + TiempoRetroceso + TiempoGiroEvasion);
    assertMovimiento(m, VelocidadAvance, VelocidadBusquedaDer);

    TEST_ASSERT_EQUAL_INT(0, g_delayCallCount);
}

void test_evadir_borde_der_gira_izquierda(void) {
    ControlMovimiento c;
    MotorMock m;

    c.ejecutar({TipoAccion::EvadirBordeDer}, m, 0);
    assertMovimiento(m, -VelocidadRetroceso, -VelocidadRetroceso);

    c.ejecutar({TipoAccion::Busqueda}, m, TiempoRetroceso);
    assertMovimiento(m, -VelocidadMaxima, VelocidadMaxima);
}

void test_evadir_borde_ambos_retrocede_mas_tiempo(void) {
    ControlMovimiento c;
    MotorMock m;

    c.ejecutar({TipoAccion::EvadirBordeAmbos}, m, 0);
    c.ejecutar({TipoAccion::Busqueda}, m, TiempoRetrocesoAmbos - 1);
    assertMovimiento(m, -VelocidadRetroceso, -VelocidadRetroceso);

    c.ejecutar({TipoAccion::Busqueda}, m, TiempoRetrocesoAmbos);
    assertMovimiento(m, VelocidadMaxima, -VelocidadMaxima);
}

void test_borde_durante_giro_reinicia_retroceso(void) {
    ControlMovimiento c;
    MotorMock m;

    c.ejecutar({TipoAccion::EvadirBordeIzq}, m, 0);
    c.ejecutar({TipoAccion::Busqueda}, m, TiempoRetroceso + 10);
    assertMovimiento(m, VelocidadMaxima, -VelocidadMaxima);

    c.ejecutar({TipoAccion::EvadirBordeDer}, m, TiempoRetroceso + 20);
    assertMovimiento(m, -VelocidadRetroceso, -VelocidadRetroceso);
}

void test_enemigo_frontal_aborta_giro_de_evasion(void) {
    ControlMovimiento c;
    MotorMock m;

    c.ejecutar({TipoAccion::EvadirBordeIzq}, m, 0);
    c.ejecutar({TipoAccion::Busqueda}, m, TiempoRetroceso + 10);
    assertMovimiento(m, VelocidadMaxima, -VelocidadMaxima);

    c.ejecutar({TipoAccion::AtaqueFrontal}, m, TiempoRetroceso + 20);
    assertMovimiento(m, VelocidadMaxima, VelocidadMaxima);
}

void test_enemigo_frontal_no_aborta_retroceso(void) {
    ControlMovimiento c;
    MotorMock m;

    c.ejecutar({TipoAccion::EvadirBordeIzq}, m, 0);
    c.ejecutar({TipoAccion::AtaqueFrontal}, m, 10);
    assertMovimiento(m, -VelocidadRetroceso, -VelocidadRetroceso);
}

int main(int, char**) {
    UNITY_BEGIN();
    RUN_TEST(test_acciones_simples_velocidades_correctas);
    RUN_TEST(test_evadir_borde_izq_retrocede_y_gira_derecha_sin_bloquear);
    RUN_TEST(test_evadir_borde_der_gira_izquierda);
    RUN_TEST(test_evadir_borde_ambos_retrocede_mas_tiempo);
    RUN_TEST(test_borde_durante_giro_reinicia_retroceso);
    RUN_TEST(test_enemigo_frontal_aborta_giro_de_evasion);
    RUN_TEST(test_enemigo_frontal_no_aborta_retroceso);
    return UNITY_END();
}
