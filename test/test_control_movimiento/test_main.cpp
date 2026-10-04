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

static void assertRetrocede(MotorMock& m) {
    assertMovimiento(m, -VelocidadRetroceso, -VelocidadRetroceso);
}

static void assertGiroEvasion(MotorMock& m, int sentido) {
    assertMovimiento(m, sentido * VelocidadGiroEvasion, -sentido * VelocidadGiroEvasion);
}

static void assertGiroBusqueda(MotorMock& m, int sentido) {
    assertMovimiento(m, sentido * VelocidadGiroBusqueda, -sentido * VelocidadGiroBusqueda);
}

static void assertAvanceBusqueda(MotorMock& m) {
    assertMovimiento(m, VelocidadAvance, VelocidadAvance);
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
    assertAccionSimple(TipoAccion::AtaqueFrontal, VelocidadAtaque, VelocidadAtaque);
    assertAccionSimple(TipoAccion::CorregirIzq, VelocidadCurva, VelocidadAtaque);
    assertAccionSimple(TipoAccion::CorregirDer, VelocidadAtaque, VelocidadCurva);
    if (GiroLateralEnRueda) {
        // La rueda del lado del enemigo se frena y hace de pivote
        assertAccionSimple(TipoAccion::DefensaIzq, 0, VelocidadRuedaPivote);
        assertAccionSimple(TipoAccion::DefensaDer, VelocidadRuedaPivote, 0);
    } else {
        assertAccionSimple(TipoAccion::DefensaIzq, -VelocidadPivoteLateral, VelocidadPivoteLateral);
        assertAccionSimple(TipoAccion::DefensaDer, VelocidadPivoteLateral, -VelocidadPivoteLateral);
    }
}

void test_busqueda_inicial_gira_y_luego_avanza(void) {
    ControlMovimiento c;
    MotorMock m;

    c.ejecutar({TipoAccion::Busqueda}, m, 1000);
    assertGiroBusqueda(m, 1);

    c.ejecutar({TipoAccion::Busqueda}, m, 1000 + TiempoGiroBusqueda - 1);
    assertGiroBusqueda(m, 1);

    c.ejecutar({TipoAccion::Busqueda}, m, 1000 + TiempoGiroBusqueda);
    assertAvanceBusqueda(m);

    c.ejecutar({TipoAccion::Busqueda}, m, 1000 + TiempoGiroBusqueda + TiempoAvanceBusqueda);
    assertGiroBusqueda(m, 1);
}

void test_busqueda_gira_hacia_el_ultimo_lado_del_enemigo(void) {
    ControlMovimiento c;
    MotorMock m;

    c.ejecutar({TipoAccion::CorregirIzq}, m, 1000);
    c.ejecutar({TipoAccion::Busqueda}, m, 1010);
    assertGiroBusqueda(m, -1);

    // Tras ver al enemigo de lado, la búsqueda empieza al acabar el giro lateral
    c.ejecutar({TipoAccion::DefensaDer}, m, 1020);
    c.ejecutar({TipoAccion::Busqueda}, m, 1020 + TiempoMaxGiroLateral);
    assertGiroBusqueda(m, 1);
}

// Al ver la línea avanzando: freno de cada rueda -> retroceso -> freno -> giro
static const unsigned long InicioRetroceso = TiempoFrenadoRuedas;
static const unsigned long InicioGiro = TiempoFrenadoRuedas + TiempoRetroceso + TiempoFrenado;

void test_evadir_borde_izq_frena_retrocede_gira_derecha_y_avanza_sin_bloquear(void) {
    ControlMovimiento c;
    MotorMock m;
    g_delayCallCount = 0;

    // Venía atacando recto: frena las dos ruedas por igual
    c.ejecutar({TipoAccion::AtaqueFrontal}, m, 1000);
    c.ejecutar({TipoAccion::EvadirBordeIzq}, m, 1000);
    assertMovimiento(m, -VelocidadMaxima, -VelocidadMaxima);

    // La línea ya no se ve, pero la maniobra continúa
    c.ejecutar({TipoAccion::Busqueda}, m, 1000 + InicioRetroceso);
    assertRetrocede(m);
    c.ejecutar({TipoAccion::Busqueda}, m, 1000 + InicioRetroceso + TiempoRetroceso - 1);
    assertRetrocede(m);

    c.ejecutar({TipoAccion::Busqueda}, m, 1000 + InicioRetroceso + TiempoRetroceso);
    assertMovimiento(m, 0, 0);

    c.ejecutar({TipoAccion::Busqueda}, m, 1000 + InicioGiro);
    assertGiroEvasion(m, 1);

    c.ejecutar({TipoAccion::Busqueda}, m, 1000 + InicioGiro + TiempoGiroEvasion - 1);
    assertGiroEvasion(m, 1);

    // Evasión completa: el robot mira hacia el centro y la búsqueda empieza avanzando
    c.ejecutar({TipoAccion::Busqueda}, m, 1000 + InicioGiro + TiempoGiroEvasion);
    assertAvanceBusqueda(m);

    TEST_ASSERT_EQUAL_INT(0, g_delayCallCount);
}

void test_evadir_borde_der_gira_izquierda(void) {
    ControlMovimiento c;
    MotorMock m;

    c.ejecutar({TipoAccion::AtaqueFrontal}, m, 0);
    c.ejecutar({TipoAccion::EvadirBordeDer}, m, 0);
    c.ejecutar({TipoAccion::Busqueda}, m, InicioGiro);
    assertGiroEvasion(m, -1);
}

void test_frenado_proporcional_a_lo_que_hacia_cada_rueda(void) {
    ControlMovimiento c;
    MotorMock m;

    // En curva: la rueda más rápida frena a tope y la otra en proporción
    c.ejecutar({TipoAccion::CorregirIzq}, m, 0);
    c.ejecutar({TipoAccion::EvadirBordeAmbos}, m, 10);
    assertMovimiento(m, -(long)VelocidadCurva * VelocidadMaxima / VelocidadAtaque, -VelocidadMaxima);
}

void test_evadir_borde_ambos_gira_hacia_el_ultimo_lado_del_enemigo(void) {
    ControlMovimiento c;
    MotorMock m;

    c.ejecutar({TipoAccion::CorregirIzq}, m, 0);
    c.ejecutar({TipoAccion::EvadirBordeAmbos}, m, 10);
    c.ejecutar({TipoAccion::Busqueda}, m, 10 + InicioRetroceso + TiempoRetrocesoAmbos - 1);
    assertRetrocede(m);

    const unsigned long giro = 10 + InicioRetroceso + TiempoRetrocesoAmbos + TiempoFrenado;
    c.ejecutar({TipoAccion::Busqueda}, m, giro);
    assertGiroEvasion(m, -1);

    c.ejecutar({TipoAccion::Busqueda}, m, giro + TiempoGiroEvasionAmbos - 1);
    assertGiroEvasion(m, -1);
}

void test_linea_girando_en_el_sitio_gira_sin_retroceder(void) {
    ControlMovimiento c;
    MotorMock m;

    // Al arrancar el robot está quieto y la búsqueda empieza girando a la derecha
    c.ejecutar({TipoAccion::Busqueda}, m, 1000);
    assertGiroBusqueda(m, 1);

    // El sensor derecho (el que va por delante) toca la línea: gira al otro lado, sin retroceder
    c.ejecutar({TipoAccion::EvadirBordeDer}, m, 1010);
    assertGiroEvasion(m, -1);

    // Nuevos toques durante el giro en el sitio no reinician la maniobra
    c.ejecutar({TipoAccion::EvadirBordeIzq}, m, 1010 + TiempoGiroEvasionEnSitio - 1);
    assertGiroEvasion(m, -1);

    c.ejecutar({TipoAccion::Busqueda}, m, 1010 + TiempoGiroEvasionEnSitio);
    assertAvanceBusqueda(m);
}

void test_linea_al_girar_justo_tras_avanzar_frena_y_retrocede(void) {
    ControlMovimiento c;
    MotorMock m;

    // Aún se desliza por la inercia del ataque: frena el giro que traía y retrocede
    c.ejecutar({TipoAccion::AtaqueFrontal}, m, 1000);
    c.ejecutar({TipoAccion::Busqueda}, m, 1010);
    c.ejecutar({TipoAccion::EvadirBordeIzq}, m, 1000 + TiempoAsentarGiro - 1);
    assertMovimiento(m, -VelocidadMaxima, VelocidadMaxima);

    c.ejecutar({TipoAccion::Busqueda}, m, 1000 + TiempoAsentarGiro - 1 + InicioRetroceso);
    assertRetrocede(m);
}

void test_linea_al_girar_tras_asentarse_gira_sin_retroceder(void) {
    ControlMovimiento c;
    MotorMock m;

    // Tras atacar, pierde al enemigo y busca girando en el sitio
    c.ejecutar({TipoAccion::AtaqueFrontal}, m, 1000);
    c.ejecutar({TipoAccion::Busqueda}, m, 1010);
    assertGiroBusqueda(m, 1);
    c.ejecutar({TipoAccion::EvadirBordeIzq}, m, 1000 + TiempoAsentarGiro);
    assertGiroEvasion(m, 1);
}

void test_enemigo_no_aborta_el_giro_minimo_de_evasion(void) {
    ControlMovimiento c;
    MotorMock m;

    c.ejecutar({TipoAccion::AtaqueFrontal}, m, 0);
    c.ejecutar({TipoAccion::EvadirBordeIzq}, m, 0);
    c.ejecutar({TipoAccion::Busqueda}, m, InicioGiro);
    assertGiroEvasion(m, 1);

    c.ejecutar({TipoAccion::AtaqueFrontal}, m, InicioGiro + TiempoMinimoGiroEvasion - 1);
    assertGiroEvasion(m, 1);

    c.ejecutar({TipoAccion::AtaqueFrontal}, m, InicioGiro + TiempoMinimoGiroEvasion);
    assertMovimiento(m, VelocidadAtaque, VelocidadAtaque);
}

void test_enemigo_lateral_aborta_giro_de_evasion(void) {
    ControlMovimiento c;
    MotorMock m;

    c.ejecutar({TipoAccion::AtaqueFrontal}, m, 0);
    c.ejecutar({TipoAccion::EvadirBordeIzq}, m, 0);
    c.ejecutar({TipoAccion::Busqueda}, m, InicioGiro);

    c.ejecutar({TipoAccion::CorregirIzq}, m, InicioGiro + TiempoMinimoGiroEvasion);
    assertMovimiento(m, VelocidadCurva, VelocidadAtaque);
}

void test_tras_giro_abortado_la_busqueda_empieza_girando(void) {
    ControlMovimiento c;
    MotorMock m;

    c.ejecutar({TipoAccion::AtaqueFrontal}, m, 0);
    c.ejecutar({TipoAccion::EvadirBordeIzq}, m, 0);
    c.ejecutar({TipoAccion::Busqueda}, m, InicioGiro);
    const unsigned long t = InicioGiro + TiempoMinimoGiroEvasion;
    c.ejecutar({TipoAccion::CorregirDer}, m, t);

    // El enemigo se pierde: el robot puede seguir de frente al borde, no debe avanzar
    c.ejecutar({TipoAccion::Busqueda}, m, t + 10);
    assertGiroBusqueda(m, 1);
}

static void assertGiroLateral(MotorMock& m, int sentido) {
    if (GiroLateralEnRueda) {
        assertMovimiento(m, sentido > 0 ? VelocidadRuedaPivote : 0, sentido > 0 ? 0 : VelocidadRuedaPivote);
    } else {
        assertMovimiento(m, sentido * VelocidadPivoteLateral, -sentido * VelocidadPivoteLateral);
    }
}

void test_giro_lateral_sigue_hasta_ver_al_enemigo_de_frente(void) {
    ControlMovimiento c;
    MotorMock m;

    c.ejecutar({TipoAccion::DefensaIzq}, m, 1000);
    assertGiroLateral(m, -1);

    // Pasa por el hueco entre sensores y por el de 45°: el giro continúa
    c.ejecutar({TipoAccion::Busqueda}, m, 1050);
    assertGiroLateral(m, -1);
    c.ejecutar({TipoAccion::CorregirIzq}, m, 1100);
    assertGiroLateral(m, -1);

    c.ejecutar({TipoAccion::AtaqueFrontal}, m, 1150);
    assertMovimiento(m, VelocidadAtaque, VelocidadAtaque);
}

void test_giro_lateral_termina_por_tiempo(void) {
    ControlMovimiento c;
    MotorMock m;

    c.ejecutar({TipoAccion::DefensaDer}, m, 1000);
    c.ejecutar({TipoAccion::Busqueda}, m, 1000 + TiempoMaxGiroLateral - 1);
    assertGiroLateral(m, 1);

    c.ejecutar({TipoAccion::Busqueda}, m, 1000 + TiempoMaxGiroLateral);
    assertGiroBusqueda(m, 1);
}

void test_linea_interrumpe_el_giro_lateral(void) {
    ControlMovimiento c;
    MotorMock m;

    c.ejecutar({TipoAccion::AtaqueFrontal}, m, 1000);
    c.ejecutar({TipoAccion::DefensaIzq}, m, 1010);
    c.ejecutar({TipoAccion::EvadirBordeIzq}, m, 1020);
    // Frena solo lo que se movía: con rueda pivote, la rueda que empujaba
    if (GiroLateralEnRueda) {
        assertMovimiento(m, 0, -VelocidadMaxima);
    }
    c.ejecutar({TipoAccion::Busqueda}, m, 1020 + InicioRetroceso);
    assertRetrocede(m);
}

void test_enemigo_frontal_no_aborta_retroceso(void) {
    ControlMovimiento c;
    MotorMock m;

    c.ejecutar({TipoAccion::AtaqueFrontal}, m, 0);
    c.ejecutar({TipoAccion::EvadirBordeIzq}, m, 0);
    c.ejecutar({TipoAccion::AtaqueFrontal}, m, InicioRetroceso + 10);
    assertRetrocede(m);
}

int main(int, char**) {
    UNITY_BEGIN();
    RUN_TEST(test_acciones_simples_velocidades_correctas);
    RUN_TEST(test_busqueda_inicial_gira_y_luego_avanza);
    RUN_TEST(test_busqueda_gira_hacia_el_ultimo_lado_del_enemigo);
    RUN_TEST(test_evadir_borde_izq_frena_retrocede_gira_derecha_y_avanza_sin_bloquear);
    RUN_TEST(test_evadir_borde_der_gira_izquierda);
    RUN_TEST(test_frenado_proporcional_a_lo_que_hacia_cada_rueda);
    RUN_TEST(test_evadir_borde_ambos_gira_hacia_el_ultimo_lado_del_enemigo);
    RUN_TEST(test_linea_girando_en_el_sitio_gira_sin_retroceder);
    RUN_TEST(test_linea_al_girar_justo_tras_avanzar_frena_y_retrocede);
    RUN_TEST(test_linea_al_girar_tras_asentarse_gira_sin_retroceder);
    RUN_TEST(test_enemigo_no_aborta_el_giro_minimo_de_evasion);
    RUN_TEST(test_enemigo_lateral_aborta_giro_de_evasion);
    RUN_TEST(test_tras_giro_abortado_la_busqueda_empieza_girando);
    RUN_TEST(test_giro_lateral_sigue_hasta_ver_al_enemigo_de_frente);
    RUN_TEST(test_giro_lateral_termina_por_tiempo);
    RUN_TEST(test_linea_interrumpe_el_giro_lateral);
    RUN_TEST(test_enemigo_frontal_no_aborta_retroceso);
    return UNITY_END();
}
